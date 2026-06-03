#include <Arduino.h>
#include <LilyGo_AMOLED.h>
#include <LV_Helper.h>
#include <lvgl.h>

// --- Hardware Libraries ---
#include <Wire.h>
#include <DHT.h> 
#include <ESP32Servo.h> 

// --- Load Our Custom Pin Configuration ---
#include "PinConfig.h" 

LilyGo_Class amoled;

// ==========================================
// HARDWARE OBJECTS
// ==========================================
Servo servoWater;

#define DHTTYPE DHT22
DHT dht(DHT_PIN, DHTTYPE);

// ==========================================
// WINGS (SERVO) LOGIC
// ==========================================
bool autoWingsEnabled = true;        
const int SOIL_THRESHOLD = 30;      
const unsigned long WINGS_TIME = 3000; 

bool isFlapping = false;
unsigned long flappingStartTime = 0;

unsigned long lastSweepUpdate = 0;
int currentServoAngle = 0;
int servoSweepDirection = 5; 

// ==========================================
// GLOBAL UI POINTERS & SENSOR STATE
// ==========================================
lv_obj_t * main_screen;
lv_obj_t * settings_overlay;

// Ladybug Parts
lv_obj_t * bug_body;
lv_obj_t * bug_head;
lv_obj_t * spots[7];
lv_anim_t breath_anim;

// Ladybug Eyes
lv_obj_t * left_eye;
lv_obj_t * right_eye;
lv_obj_t * left_pupil;
lv_obj_t * right_pupil;

// Dashboard Interactive Elements
lv_obj_t * btn_wings;
lv_obj_t * lbl_soil_val;
lv_obj_t * lbl_temp_val;
lv_obj_t * lbl_hum_val;

// Popup Menu Elements
lv_obj_t * info_popup;
lv_obj_t * popup_title;
lv_obj_t * popup_icon;
lv_obj_t * popup_value;
lv_obj_t * popup_desc;

// Live Sensor Data Storage
int current_soil = 0;
float current_temp = 0.0;
float current_hum = 0.0;
unsigned long lastSensorUpdate = 0;

// ==========================================
// SERVO FUNCTIONS
// ==========================================
void startFlapping() {
    Serial.println("🦋 WINGS INITIATED! Starting sweep.");
    isFlapping = true;
    flappingStartTime = millis();
    currentServoAngle = 0;
    servoSweepDirection = 5; 
    lv_obj_set_style_bg_color(btn_wings, lv_color_hex(0x551199), 0); 
}

void stopFlapping() {
    Serial.println("🛑 WINGS COMPLETE. Servo parked.");
    servoWater.write(0); 
    isFlapping = false;
    lv_obj_set_style_bg_color(btn_wings, lv_color_hex(0x9933FF), 0); 
}

// ==========================================
// MAIN DASHBOARD BUTTON CALLBACKS
// ==========================================
void manual_wings_cb(lv_event_t * e) {
    if (!isFlapping) { startFlapping(); }
}

void close_popup_cb(lv_event_t * e) {
    lv_obj_add_flag(info_popup, LV_OBJ_FLAG_HIDDEN);
}

void soil_click_cb(lv_event_t * e) {
    lv_label_set_text(popup_title, "SOIL MOISTURE");
    lv_label_set_text(popup_icon, LV_SYMBOL_TINT);
    lv_obj_set_style_text_color(popup_icon, lv_color_hex(0x0088FF), 0);
    
    char buf[32];
    sprintf(buf, "%d %%", current_soil);
    lv_label_set_text(popup_value, buf);
    
    lv_label_set_text(popup_desc, "Basil loves moist, well-drained soil.\nKeep it between 40% and 60%.\nNever let it completely dry out!");
    lv_obj_clear_flag(info_popup, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(info_popup); 
}

void temp_click_cb(lv_event_t * e) {
    lv_label_set_text(popup_title, "TEMPERATURE");
    lv_label_set_text(popup_icon, LV_SYMBOL_HOME);
    lv_obj_set_style_text_color(popup_icon, lv_color_hex(0xFF4444), 0);
    
    char buf[32];
    sprintf(buf, "%.1f C", current_temp);
    lv_label_set_text(popup_value, buf);
    
    lv_label_set_text(popup_desc, "Basil thrives in a warm climate\nbetween 20 C and 25 C.\nCold drafts will blacken the leaves.");
    lv_obj_clear_flag(info_popup, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(info_popup);
}

void hum_click_cb(lv_event_t * e) {
    lv_label_set_text(popup_title, "AIR HUMIDITY");
    lv_label_set_text(popup_icon, LV_SYMBOL_REFRESH);
    lv_obj_set_style_text_color(popup_icon, lv_color_hex(0x22CC44), 0);
    
    char buf[32];
    sprintf(buf, "%.0f %%", current_hum);
    lv_label_set_text(popup_value, buf);
    
    lv_label_set_text(popup_desc, "Ideal humidity is 40% to 60%.\nToo high risks mold and rot,\ntoo low crisping leaves.");
    lv_obj_clear_flag(info_popup, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(info_popup);
}

void open_settings_cb(lv_event_t * e) {
    lv_obj_clear_flag(settings_overlay, LV_OBJ_FLAG_HIDDEN);
}

void close_settings_cb(lv_event_t * e) {
    lv_obj_add_flag(settings_overlay, LV_OBJ_FLAG_HIDDEN);
}

void breath_anim_cb(void * var, int32_t v) {
    lv_obj_set_style_translate_y((lv_obj_t *)var, v, 0);
}

// ==========================================
// SETTINGS MENU CALLBACKS
// ==========================================

void lib_click_cb(lv_event_t * e) {
    lv_label_set_text(popup_title, "PLANT LIBRARY");
    lv_label_set_text(popup_icon, LV_SYMBOL_DIRECTORY);
    lv_obj_set_style_text_color(popup_icon, lv_color_hex(0x22CC44), 0);
    lv_label_set_text(popup_value, "BASIL SELECTED");
    lv_label_set_text(popup_desc, "Connect to the mobile app\nto upload new plant profiles\nand custom thresholds.");
    lv_obj_clear_flag(info_popup, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(info_popup);
}

void toggle_wings_cb(lv_event_t * e) {
    lv_obj_t * btn = lv_event_get_target(e);
    bool is_checked = lv_obj_has_state(btn, LV_STATE_CHECKED);
    if(is_checked) { lv_obj_set_style_bg_color(btn, lv_color_hex(0x22CC44), 0); } 
    else { lv_obj_set_style_bg_color(btn, lv_color_hex(0x333333), 0); }
    autoWingsEnabled = is_checked;
}

void wifi_click_cb(lv_event_t * e) {
    lv_label_set_text(popup_title, "WIFI SETTINGS");
    lv_label_set_text(popup_icon, LV_SYMBOL_WIFI);
    lv_obj_set_style_text_color(popup_icon, lv_color_hex(0xFFAA00), 0);
    lv_label_set_text(popup_value, "DISCONNECTED");
    lv_label_set_text(popup_desc, "Device is in offline mode.\nBluetooth provisioning\ncoming in next update.");
    lv_obj_clear_flag(info_popup, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(info_popup);
}

void toggle_sleep_cb(lv_event_t * e) {
    lv_obj_t * btn = lv_event_get_target(e);
    bool is_checked = lv_obj_has_state(btn, LV_STATE_CHECKED);
    if(is_checked) { 
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x22CC44), 0); 
        amoled.setBrightness(20); // Dim screen significantly
    } else { 
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x333333), 0); 
        amoled.setBrightness(180); // Restore brightness
    }
}

void data_click_cb(lv_event_t * e) {
    lv_label_set_text(popup_title, "CALIBRATION DATA");
    lv_label_set_text(popup_icon, LV_SYMBOL_SETTINGS);
    lv_obj_set_style_text_color(popup_icon, lv_color_hex(0xFFFFFF), 0);
    
    int raw = analogRead(SOIL_PIN);
    char buf[64];
    sprintf(buf, "Raw Soil Input: %d\nDry Limit: %d\nWet Limit: %d", raw, SOIL_DRY_VAL, SOIL_WET_VAL);
    
    lv_label_set_text(popup_value, "SENSOR DEBUG");
    lv_label_set_text(popup_desc, buf);
    lv_obj_clear_flag(info_popup, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(info_popup);
}

void system_click_cb(lv_event_t * e) {
    lv_label_set_text(popup_title, "SYSTEM INFO");
    lv_label_set_text(popup_icon, LV_SYMBOL_SETTINGS);
    lv_obj_set_style_text_color(popup_icon, lv_color_hex(0x0088FF), 0);
    lv_label_set_text(popup_value, "HEXULE OS v1.0");
    lv_label_set_text(popup_desc, "Hardware: LilyGO T4 S3\nDirect Sensor Polling Active\nAll systems operational.");
    lv_obj_clear_flag(info_popup, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(info_popup);
}


void setup() {
  Serial.begin(115200);
  delay(1000); 

  // --- 1. INITIALIZE AMOLED & TOUCH ---
  if (!amoled.begin()) {
    while (1) { Serial.println("❌ ERROR: Display failed!"); delay(1000); }
  }
  amoled.setBrightness(180); 
  beginLvglHelper(amoled);

  // --- 2. INITIALIZE SENSORS & SERVO ---
  dht.begin();
  pinMode(SOIL_PIN, INPUT);

  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  
  servoWater.setPeriodHertz(50); 
  servoWater.attach(SERVO_WATER_PIN, 500, 2400);
  servoWater.write(0);

  main_screen = lv_scr_act();
  lv_obj_set_style_bg_color(main_screen, lv_color_hex(0x000000), 0);

  /* =========================================
   * LADYBUG CENTER GRAPHIC
   * ========================================= */
  lv_obj_t * gear_btn = lv_btn_create(main_screen);
  lv_obj_set_size(gear_btn, 80, 80);
  lv_obj_align(gear_btn, LV_ALIGN_TOP_MID, 0, 10);
  lv_obj_set_style_radius(gear_btn, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_color(gear_btn, lv_color_hex(0x222222), 0); 
  lv_obj_add_event_cb(gear_btn, open_settings_cb, LV_EVENT_CLICKED, NULL);

  lv_obj_t * gear_label = lv_label_create(gear_btn);
  lv_label_set_text(gear_label, LV_SYMBOL_SETTINGS);
  lv_obj_center(gear_label);

  bug_body = lv_obj_create(main_screen);
  lv_obj_set_size(bug_body, 220, 220); 
  lv_obj_align(bug_body, LV_ALIGN_CENTER, 0, 40); 
  lv_obj_set_style_radius(bug_body, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_color(bug_body, lv_color_hex(0xFF1111), 0); 
  lv_obj_set_style_border_width(bug_body, 0, 0);

  bug_head = lv_obj_create(main_screen);
  lv_obj_set_size(bug_head, 120, 80); 
  lv_obj_align_to(bug_head, bug_body, LV_ALIGN_OUT_TOP_MID, 0, 40);
  lv_obj_set_style_radius(bug_head, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_color(bug_head, lv_color_hex(0x111111), 0);
  lv_obj_set_style_border_width(bug_head, 0, 0);

  left_eye = lv_obj_create(bug_head);
  lv_obj_set_size(left_eye, 28, 28); 
  lv_obj_set_style_radius(left_eye, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_color(left_eye, lv_color_hex(0xFFFFFF), 0); 
  lv_obj_set_style_border_width(left_eye, 0, 0);
  lv_obj_align(left_eye, LV_ALIGN_CENTER, -25, -10); 

  right_eye = lv_obj_create(bug_head);
  lv_obj_set_size(right_eye, 28, 28); 
  lv_obj_set_style_radius(right_eye, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_color(right_eye, lv_color_hex(0xFFFFFF), 0); 
  lv_obj_set_style_border_width(right_eye, 0, 0);
  lv_obj_align(right_eye, LV_ALIGN_CENTER, 25, -10); 

  left_pupil = lv_obj_create(left_eye);
  lv_obj_set_size(left_pupil, 10, 10); 
  lv_obj_set_style_radius(left_pupil, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_color(left_pupil, lv_color_hex(0x000000), 0); 
  lv_obj_set_style_border_width(left_pupil, 0, 0);
  lv_obj_align(left_pupil, LV_ALIGN_CENTER, 0, 0); 

  right_pupil = lv_obj_create(right_eye);
  lv_obj_set_size(right_pupil, 10, 10); 
  lv_obj_set_style_radius(right_pupil, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_color(right_pupil, lv_color_hex(0x000000), 0); 
  lv_obj_set_style_border_width(right_pupil, 0, 0);
  lv_obj_align(right_pupil, LV_ALIGN_CENTER, 0, 0); 

  for(int i = 0; i < 7; i++) {
      spots[i] = lv_obj_create(bug_body);
      lv_obj_set_size(spots[i], 36, 36); 
      lv_obj_set_style_radius(spots[i], LV_RADIUS_CIRCLE, 0);
      lv_obj_set_style_bg_color(spots[i], lv_color_hex(0x000000), 0);
      lv_obj_set_style_border_width(spots[i], 0, 0);
  }
  lv_obj_align(spots[0], LV_ALIGN_CENTER, 0, -15);     
  lv_obj_align(spots[1], LV_ALIGN_CENTER, -50, -55);   
  lv_obj_align(spots[2], LV_ALIGN_CENTER, 50, -55);    
  lv_obj_align(spots[3], LV_ALIGN_CENTER, -75, 20);    
  lv_obj_align(spots[4], LV_ALIGN_CENTER, 75, 20);     
  lv_obj_align(spots[5], LV_ALIGN_CENTER, -40, 75);    
  lv_obj_align(spots[6], LV_ALIGN_CENTER, 40, 75);     

  lv_anim_init(&breath_anim);
  lv_anim_set_var(&breath_anim, bug_body);
  lv_anim_set_values(&breath_anim, -8, 8); 
  lv_anim_set_time(&breath_anim, 2500); 
  lv_anim_set_playback_time(&breath_anim, 2500); 
  lv_anim_set_repeat_count(&breath_anim, LV_ANIM_REPEAT_INFINITE);
  lv_anim_set_exec_cb(&breath_anim, breath_anim_cb);
  lv_anim_set_path_cb(&breath_anim, lv_anim_path_ease_in_out);
  lv_anim_start(&breath_anim);

  lv_obj_t * plant_name = lv_label_create(main_screen);
  lv_label_set_text(plant_name, "BASIL BERRY");
  lv_obj_set_style_text_font(plant_name, &lv_font_montserrat_24, 0); 
  lv_obj_set_style_text_color(plant_name, lv_color_hex(0xFFFFFF), 0);
  lv_obj_align(plant_name, LV_ALIGN_BOTTOM_MID, 0, -20);

  /* =========================================
   * BIG DASHBOARD BUTTONS (CIRCLES SCALED TO 150)
   * ========================================= */

  // 1. TOP LEFT: WINGS
  btn_wings = lv_btn_create(main_screen);
  lv_obj_set_size(btn_wings, 150, 150);
  lv_obj_align(btn_wings, LV_ALIGN_TOP_LEFT, 20, 20); 
  lv_obj_set_style_radius(btn_wings, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_color(btn_wings, lv_color_hex(0x9933FF), 0); 
  lv_obj_add_event_cb(btn_wings, manual_wings_cb, LV_EVENT_CLICKED, NULL);

  lv_obj_t * lbl_wings_icon = lv_label_create(btn_wings);
  lv_label_set_text(lbl_wings_icon, LV_SYMBOL_UP); 
  lv_obj_align(lbl_wings_icon, LV_ALIGN_TOP_MID, 0, 20);

  lv_obj_t * lbl_wings_text = lv_label_create(btn_wings);
  lv_obj_set_style_text_font(lbl_wings_text, &lv_font_montserrat_24, 0); 
  lv_label_set_text(lbl_wings_text, "WINGS");
  lv_obj_align(lbl_wings_text, LV_ALIGN_BOTTOM_MID, 0, -25);


  // 2. TOP RIGHT: SOIL MOISTURE
  lv_obj_t * btn_soil = lv_btn_create(main_screen);
  lv_obj_set_size(btn_soil, 150, 150);
  lv_obj_align(btn_soil, LV_ALIGN_TOP_RIGHT, -20, 20); 
  lv_obj_set_style_radius(btn_soil, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_color(btn_soil, lv_color_hex(0x0088FF), 0); 
  lv_obj_add_event_cb(btn_soil, soil_click_cb, LV_EVENT_CLICKED, NULL); 
  
  lv_obj_t * lbl_soil_icon = lv_label_create(btn_soil);
  lv_label_set_text(lbl_soil_icon, LV_SYMBOL_TINT); 
  lv_obj_align(lbl_soil_icon, LV_ALIGN_TOP_MID, 0, 20);

  lbl_soil_val = lv_label_create(btn_soil);
  lv_obj_set_style_text_font(lbl_soil_val, &lv_font_montserrat_24, 0);
  lv_label_set_text(lbl_soil_val, "--"); 
  lv_obj_align(lbl_soil_val, LV_ALIGN_BOTTOM_MID, 0, -25);


  // 3. BOTTOM LEFT: TEMPERATURE
  lv_obj_t * btn_temp = lv_btn_create(main_screen);
  lv_obj_set_size(btn_temp, 150, 150);
  lv_obj_align(btn_temp, LV_ALIGN_BOTTOM_LEFT, 20, -20); 
  lv_obj_set_style_radius(btn_temp, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_color(btn_temp, lv_color_hex(0xFF4444), 0); 
  lv_obj_add_event_cb(btn_temp, temp_click_cb, LV_EVENT_CLICKED, NULL); 
  
  lv_obj_t * lbl_temp_icon = lv_label_create(btn_temp);
  lv_label_set_text(lbl_temp_icon, LV_SYMBOL_HOME);
  lv_obj_align(lbl_temp_icon, LV_ALIGN_TOP_MID, 0, 20);

  lbl_temp_val = lv_label_create(btn_temp);
  lv_obj_set_style_text_font(lbl_temp_val, &lv_font_montserrat_24, 0);
  lv_label_set_text(lbl_temp_val, "--");
  lv_obj_align(lbl_temp_val, LV_ALIGN_BOTTOM_MID, 0, -25);


  // 4. BOTTOM RIGHT: HUMIDITY
  lv_obj_t * btn_hum = lv_btn_create(main_screen);
  lv_obj_set_size(btn_hum, 150, 150);
  lv_obj_align(btn_hum, LV_ALIGN_BOTTOM_RIGHT, -20, -20); 
  lv_obj_set_style_radius(btn_hum, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_bg_color(btn_hum, lv_color_hex(0x22CC44), 0); 
  lv_obj_add_event_cb(btn_hum, hum_click_cb, LV_EVENT_CLICKED, NULL); 
  
  lv_obj_t * lbl_hum_icon = lv_label_create(btn_hum);
  lv_label_set_text(lbl_hum_icon, LV_SYMBOL_REFRESH); 
  lv_obj_align(lbl_hum_icon, LV_ALIGN_TOP_MID, 0, 20);

  lbl_hum_val = lv_label_create(btn_hum);
  lv_obj_set_style_text_font(lbl_hum_val, &lv_font_montserrat_24, 0);
  lv_label_set_text(lbl_hum_val, "--"); 
  lv_obj_align(lbl_hum_val, LV_ALIGN_BOTTOM_MID, 0, -25);

  /* =========================================
   * INFO APP POPUP (NOW WITH LARGE TEXT & SCALED UP)
   * ========================================= */
  info_popup = lv_obj_create(main_screen);
  lv_obj_set_size(info_popup, 480, 360); 
  lv_obj_center(info_popup);
  lv_obj_set_style_bg_color(info_popup, lv_color_hex(0x1A1A1A), 0); 
  lv_obj_set_style_border_color(info_popup, lv_color_hex(0x444444), 0);
  lv_obj_set_style_border_width(info_popup, 2, 0);
  lv_obj_set_style_radius(info_popup, 30, 0);
  lv_obj_add_flag(info_popup, LV_OBJ_FLAG_HIDDEN); 

  popup_title = lv_label_create(info_popup);
  lv_obj_set_style_text_font(popup_title, &lv_font_montserrat_24, 0); 
  lv_obj_set_style_text_color(popup_title, lv_color_hex(0xAAAAAA), 0);
  lv_obj_align(popup_title, LV_ALIGN_TOP_MID, 0, 15);

  popup_icon = lv_label_create(info_popup);
  lv_obj_set_style_text_font(popup_icon, &lv_font_montserrat_24, 0); 
  lv_obj_align(popup_icon, LV_ALIGN_TOP_MID, 0, 60);

  popup_value = lv_label_create(info_popup);
  lv_obj_set_style_text_font(popup_value, &lv_font_montserrat_24, 0); 
  lv_obj_set_style_text_color(popup_value, lv_color_hex(0xFFFFFF), 0);
  lv_obj_align(popup_value, LV_ALIGN_TOP_MID, 0, 100);

  popup_desc = lv_label_create(info_popup);
  lv_obj_set_style_text_font(popup_desc, &lv_font_montserrat_24, 0); 
  lv_obj_set_width(popup_desc, 440); 
  lv_label_set_long_mode(popup_desc, LV_LABEL_LONG_WRAP);
  lv_obj_set_style_text_align(popup_desc, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_set_style_text_color(popup_desc, lv_color_hex(0xDDDDDD), 0);
  lv_obj_align(popup_desc, LV_ALIGN_CENTER, 0, 15);

  lv_obj_t * close_btn = lv_btn_create(info_popup);
  lv_obj_set_size(close_btn, 180, 60);
  lv_obj_align(close_btn, LV_ALIGN_BOTTOM_MID, 0, -15);
  lv_obj_set_style_bg_color(close_btn, lv_color_hex(0x333333), 0);
  lv_obj_set_style_radius(close_btn, 25, 0);
  lv_obj_add_event_cb(close_btn, close_popup_cb, LV_EVENT_CLICKED, NULL);
  
  lv_obj_t * close_lbl = lv_label_create(close_btn);
  lv_obj_set_style_text_font(close_lbl, &lv_font_montserrat_24, 0); 
  lv_label_set_text(close_lbl, "CLOSE");
  lv_obj_center(close_lbl);

  /* =========================================
   * SETTINGS OVERLAY
   * ========================================= */
  settings_overlay = lv_obj_create(main_screen);
  lv_obj_set_size(settings_overlay, LV_PCT(100), LV_PCT(100)); 
  lv_obj_set_style_bg_color(settings_overlay, lv_color_hex(0x050505), 0); 
  lv_obj_set_style_bg_opa(settings_overlay, LV_OPA_90, 0); 
  lv_obj_set_style_border_width(settings_overlay, 0, 0);
  lv_obj_add_flag(settings_overlay, LV_OBJ_FLAG_HIDDEN); 

  lv_obj_t * grid_container = lv_obj_create(settings_overlay);
  lv_obj_set_size(grid_container, LV_PCT(100), 400); 
  lv_obj_align(grid_container, LV_ALIGN_TOP_MID, 0, 40);
  lv_obj_set_style_bg_opa(grid_container, LV_OPA_TRANSP, 0);
  lv_obj_set_style_border_width(grid_container, 0, 0);
  lv_obj_set_flex_flow(grid_container, LV_FLEX_FLOW_ROW_WRAP); 
  lv_obj_set_flex_align(grid_container, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  // Define buttons and callbacks manually instead of a basic loop
  const char* btn_icons[] = {LV_SYMBOL_DIRECTORY, LV_SYMBOL_UP, LV_SYMBOL_WIFI, LV_SYMBOL_POWER, LV_SYMBOL_IMAGE, LV_SYMBOL_SETTINGS};
  const char* btn_texts[] = {"LIBRARY", "AUTO WINGS", "WIFI", "SLEEP", "DATA", "SYSTEM"};
  
  lv_obj_t * menu_btns[6];

  for(int i = 0; i < 6; i++) {
      menu_btns[i] = lv_btn_create(grid_container);
      lv_obj_set_size(menu_btns[i], 190, 140); 
      lv_obj_set_style_bg_color(menu_btns[i], lv_color_hex(0x333333), 0); 
      lv_obj_set_style_radius(menu_btns[i], 25, 0);

      lv_obj_t * menu_lbl = lv_label_create(menu_btns[i]);
      lv_obj_set_style_text_font(menu_lbl, &lv_font_montserrat_24, 0);
      char buf[32];
      sprintf(buf, "%s\n%s", btn_icons[i], btn_texts[i]);
      lv_label_set_text(menu_lbl, buf);
      lv_obj_set_style_text_align(menu_lbl, LV_TEXT_ALIGN_CENTER, 0);
      lv_obj_center(menu_lbl);
  }

  // Assign specific functionality to each settings button
  lv_obj_add_event_cb(menu_btns[0], lib_click_cb, LV_EVENT_CLICKED, NULL);

  lv_obj_add_flag(menu_btns[1], LV_OBJ_FLAG_CHECKABLE);
  lv_obj_add_state(menu_btns[1], LV_STATE_CHECKED);
  lv_obj_set_style_bg_color(menu_btns[1], lv_color_hex(0x22CC44), 0);
  lv_obj_add_event_cb(menu_btns[1], toggle_wings_cb, LV_EVENT_VALUE_CHANGED, NULL);

  lv_obj_add_event_cb(menu_btns[2], wifi_click_cb, LV_EVENT_CLICKED, NULL);

  lv_obj_add_flag(menu_btns[3], LV_OBJ_FLAG_CHECKABLE);
  lv_obj_add_event_cb(menu_btns[3], toggle_sleep_cb, LV_EVENT_VALUE_CHANGED, NULL);

  lv_obj_add_event_cb(menu_btns[4], data_click_cb, LV_EVENT_CLICKED, NULL);
  lv_obj_add_event_cb(menu_btns[5], system_click_cb, LV_EVENT_CLICKED, NULL);

  // Settings Menu "Back" Button
  lv_obj_t * back_btn = lv_btn_create(settings_overlay);
  lv_obj_set_size(back_btn, 280, 80); 
  lv_obj_align(back_btn, LV_ALIGN_BOTTOM_MID, 0, -30);
  lv_obj_set_style_bg_color(back_btn, lv_color_hex(0xFF1111), 0); 
  lv_obj_set_style_radius(back_btn, 40, 0); 
  lv_obj_add_event_cb(back_btn, close_settings_cb, LV_EVENT_CLICKED, NULL);

  lv_obj_t * back_lbl = lv_label_create(back_btn);
  lv_obj_set_style_text_font(back_lbl, &lv_font_montserrat_24, 0);
  lv_label_set_text(back_lbl, "BACK TO BUG");
  lv_obj_center(back_lbl);
  
  Serial.println("✅ Setup complete! UI Scaled and System ready.\n");
}


void loop() {
  lv_timer_handler();
  delay(5); 

  // --- CHECK NON-BLOCKING WINGS SWEEP LOGIC ---
  if (isFlapping) {
      if (millis() - flappingStartTime >= WINGS_TIME) {
          stopFlapping();
      } else {
          if (millis() - lastSweepUpdate > 30) { 
              lastSweepUpdate = millis();
              currentServoAngle += servoSweepDirection;
              
              if (currentServoAngle >= 90) {
                  currentServoAngle = 90;
                  servoSweepDirection = -5; 
              } else if (currentServoAngle <= 0) {
                  currentServoAngle = 0;
                  servoSweepDirection = 5;  
              }
              servoWater.write(currentServoAngle);
          }
      }
  }

  // --- SENSOR POLLING (Every 3 seconds) ---
  if (millis() - lastSensorUpdate > 3000) {
      lastSensorUpdate = millis();
      char buf[32]; 

      // 1. Read Soil Moisture 
      int rawSoil = analogRead(SOIL_PIN);
      current_soil = map(rawSoil, SOIL_DRY_VAL, SOIL_WET_VAL, 0, 100); 
      current_soil = constrain(current_soil, 0, 100); 
      
      sprintf(buf, "%d%%", current_soil);
      lv_label_set_text(lbl_soil_val, buf);

      if (autoWingsEnabled && !isFlapping && current_soil < SOIL_THRESHOLD) {
          Serial.println("⚠️ Soil is too dry! Auto-wings triggered.");
          startFlapping();
      }

      // 2. Read DHT22 Temperature & Humidity 
      current_hum = dht.readHumidity();
      current_temp = dht.readTemperature();

      if (!isnan(current_hum) && !isnan(current_temp)) {
          sprintf(buf, "%.1fC", current_temp);
          lv_label_set_text(lbl_temp_val, buf);
          
          sprintf(buf, "%.0f%%", current_hum); 
          lv_label_set_text(lbl_hum_val, buf);
      } 
  }
}