#ifndef WEBDASHBOARD_H
#define WEBDASHBOARD_H

#include <Arduino.h>

const char* dashboard_html_template = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
    <title>Florette</title>
    
    <meta name="apple-mobile-web-app-capable" content="yes">
    <meta name="apple-mobile-web-app-status-bar-style" content="black-translucent">
    <link rel="manifest" href='data:application/manifest+json,{"name":"Florette Sync","short_name":"Florette","start_url":".","display":"standalone","background_color":"#050505","theme_color":"#050505","icons":[{"src":"data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHZpZXdCb3g9IjAgMCAxMDAgMTAwIj48Y2lyY2xlIGN4PSI1MCIgY3k9IjUwIiByPSI1MCIgZmlsbD0iI2ZmMTExMSIvPjwvc3ZnPg==","sizes":"192x192","type":"image/svg+xml"}]}'>
    
    <link href="https://api.fontshare.com/v2/css?f[]=satoshi@700,500,400&display=swap" rel="stylesheet">
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    
    <style>
        :root {
            --glass-bg: rgba(255, 255, 255, 0.03);
            --glass-border: rgba(255, 255, 255, 0.06);
            --text-main: #ffffff;
            --text-dim: #999999;
        }

        body {
            font-family: 'Satoshi', sans-serif;
            margin: 0; padding: 0;
            background-color: #050505;
            color: var(--text-main);
            overflow-x: hidden;
            transition: background 1.5s ease;
        }

        #ambient-bg {
            position: fixed; top: 0; left: 0; width: 100vw; height: 100vh;
            z-index: -1;
            background: radial-gradient(circle at 50% 0%, hsla(160, 40%, 15%, 0.5), transparent 70%);
            transition: background 1.5s ease;
        }

        header {
            display: flex; justify-content: space-between; align-items: center;
            padding: 20px 40px;
            border-bottom: 1px solid var(--glass-border);
            background: rgba(0,0,0,0.2); backdrop-filter: blur(20px);
            position: sticky; top: 0; z-index: 100;
        }

        .logo { font-weight: 700; font-size: 1.5rem; letter-spacing: 1px; display: flex; align-items: center; gap: 10px; }
        .status-dot { width: 10px; height: 10px; border-radius: 50%; background: #22CC44; box-shadow: 0 0 10px #22CC44; }

        .container {
            max-width: 1200px; margin: 40px auto; padding: 0 20px;
            display: grid; grid-template-columns: 1fr 2fr; gap: 40px;
        }

        @media (max-width: 800px) {
            .container { grid-template-columns: 1fr; }
            header { padding: 20px; }
        }

        .glass-panel {
            background: var(--glass-bg); backdrop-filter: blur(20px); -webkit-backdrop-filter: blur(20px);
            border: 1px solid var(--glass-border); border-radius: 24px;
            padding: 30px;
        }

        .section-title { font-size: 0.9rem; text-transform: uppercase; letter-spacing: 2px; color: var(--text-dim); margin-bottom: 20px; font-weight: 700;}

        /* Plant Library Select Menu */
        .glass-select {
            background: rgba(0, 0, 0, 0.4);
            border: 1px solid var(--glass-border);
            color: white;
            padding: 8px 12px;
            border-radius: 12px;
            font-family: inherit;
            font-size: 0.9rem;
            outline: none;
            cursor: pointer;
            backdrop-filter: blur(10px);
        }
        .glass-select option { background: #111; }
        
        .alarm-card {
            background: rgba(255, 170, 0, 0.05);
            border: 1px solid rgba(255, 170, 0, 0.2);
            border-radius: 12px;
            padding: 15px;
            margin-top: 15px;
            font-size: 0.85rem;
            color: #ffcc66;
            line-height: 1.6;
        }

        /* Task Management Checklist */
        .task-list { display: flex; flex-direction: column; gap: 12px; }
        .task-item { 
            display: flex; align-items: center; gap: 15px; 
            background: rgba(255,255,255,0.02); padding: 14px 18px; 
            border-radius: 12px; border: 1px solid var(--glass-border);
            cursor: pointer; transition: 0.2s; user-select: none;
        }
        .task-item:hover { background: rgba(255,255,255,0.05); }
        .task-item input { display: none; }
        .checkmark { 
            width: 22px; height: 22px; border-radius: 6px; 
            border: 2px solid var(--text-dim); display: flex; align-items: center; justify-content: center;
            transition: 0.2s;
        }
        .task-item input:checked ~ .checkmark { background: #9933FF; border-color: #9933FF; }
        .task-item input:checked ~ .checkmark::after { content: '✔'; color: white; font-size: 14px; font-weight: 700;}
        .task-item input:checked ~ .task-text { text-decoration: line-through; color: var(--text-dim); }
        .task-text { font-size: 0.95rem; font-weight: 500; transition: 0.2s; }

        .chart-container { position: relative; height: 250px; width: 100%; }

        .sensor-grid { display: grid; grid-template-columns: 1fr 1fr 1fr; gap: 15px; margin-bottom: 20px; }
        .metric p { margin: 0; font-size: 0.9rem; color: var(--text-dim); }
        .metric h2 { margin: 5px 0 0 0; font-size: 1.8rem; font-weight: 700; }
        @media (max-width: 600px) { .sensor-grid { grid-template-columns: 1fr; } }

        .timeline { display: flex; flex-direction: column; gap: 10px; margin-top: 20px; border-top: 1px solid var(--glass-border); padding-top: 20px;}
        .pill { display: flex; justify-content: space-between; font-size: 0.95rem; }
        .pill span { color: var(--text-dim); }

        .controls { display: grid; grid-template-columns: 1fr; gap: 10px; margin-top: 30px; }
        .btn {
            background: rgba(255, 255, 255, 0.02); border: 1px solid var(--glass-border);
            border-radius: 12px; padding: 16px; color: white;
            font-family: inherit; font-weight: 500; font-size: 1rem; cursor: pointer; transition: 0.2s;
        }
        .btn:hover { background: rgba(255, 255, 255, 0.08); }
        .btn-water { border-color: rgba(0, 136, 255, 0.5); color: #88ccff; }
        .btn-temp { border-color: rgba(255, 68, 68, 0.5); color: #ff8888; }
        
        /* --- HIGH-FIDELITY CSS LADYBUG --- */
        .bug-container { position: relative; width: 100px; height: 130px; margin: 15px auto 35px; perspective: 1000px; }
        
        .bug-head { 
            position: absolute; top: 0; left: 50%; transform: translateX(-50%); 
            width: 36px; height: 28px; background: #111; 
            border-radius: 40px 40px 15px 15px; z-index: 2; 
        }
        
        /* Antennae */
        .bug-head::before { content: ''; position: absolute; top: -12px; left: 6px; width: 2px; height: 14px; background: #111; transform: rotate(-25deg); border-radius: 2px;}
        .bug-head::after { content: ''; position: absolute; top: -12px; right: 6px; width: 2px; height: 14px; background: #111; transform: rotate(25deg); border-radius: 2px;}

        .bug-body { 
            position: absolute; bottom: 0; left: 50%; transform: translateX(-50%); 
            width: 100px; height: 105px; background: #111; 
            border-radius: 50%; z-index: 1; 
            box-shadow: 0 10px 20px rgba(0,0,0,0.5);
        }
        
        .wing { 
            position: absolute; top: 15px; width: 50px; height: 106px; 
            background: linear-gradient(135deg, #FF2A2A 0%, #A60000 100%); 
            z-index: 3; transition: transform 0.3s ease; 
            box-shadow: inset -3px -3px 10px rgba(0,0,0,0.4); 
            overflow: hidden; /* Clips the spots inside the wings */
        }
        
        .wing-left { 
            left: 0; border-radius: 50px 5px 15px 50px; 
            transform-origin: 90% 10%; border-right: 1px solid rgba(0,0,0,0.6); 
        }
        
        .wing-right { 
            right: 0; border-radius: 5px 50px 50px 15px; 
            transform-origin: 10% 10%; border-left: 1px solid rgba(0,0,0,0.6); 
        }

        .spot { position: absolute; background: #111; border-radius: 50%; }

        .flap-normal .wing-left { animation: normal-L 1.5s ease-in-out infinite alternate; }
        .flap-normal .wing-right { animation: normal-R 1.5s ease-in-out infinite alternate; }
        
        /* Fixed Signs: Left wing rotates clockwise, Right wing rotates counter-clockwise */
        @keyframes normal-L { 0% { transform: rotate(0deg); } 100% { transform: rotate(65deg); } }
        @keyframes normal-R { 0% { transform: rotate(0deg); } 100% { transform: rotate(-65deg); } }
        
        .flap-temp .wing-left { animation: temp-L 0.15s linear infinite alternate; }
        .flap-temp .wing-right { animation: temp-R 0.15s linear infinite alternate; }
        
        @keyframes temp-L { 0% { transform: rotate(0deg); } 100% { transform: rotate(75deg); } }
        @keyframes temp-R { 0% { transform: rotate(0deg); } 100% { transform: rotate(-75deg); } }
        
        .flap-water .wing-left { animation: water-L 2s ease-in-out infinite; }
        .flap-water .wing-right { animation: water-R 2s ease-in-out infinite; }
        
        @keyframes water-L { 0%, 100% { transform: rotate(0deg); } 20% { transform: rotate(60deg); } 40%, 60% { transform: rotate(45deg); } 80% { transform: rotate(60deg); } }
        @keyframes water-R { 0%, 100% { transform: rotate(0deg); } 20% { transform: rotate(-60deg); } 40%, 60% { transform: rotate(-45deg); } 80% { transform: rotate(-60deg); } }
    </style>
</head>
<body>
    <div id="ambient-bg"></div>

    <header>
        <div class="logo">🐞 Florette</div>
        <div style="display: flex; align-items: center; gap: 8px;">
            <div class="status-dot" id="net-status"></div>
            <span style="font-size: 0.9rem; color: var(--text-dim);">Live Sync</span>
        </div>
    </header>

    <div class="container">
        <aside>
            <div class="glass-panel" style="margin-bottom: 20px;">
                <div class="section-title" style="text-align: center;">Companion Status</div>
                <div class="bug-container" id="ladybug">
                    <div class="bug-head"></div>
                    <div class="bug-body"></div>
                    <div class="wing wing-left">
                        <div class="spot" style="width:14px; height:14px; top:15px; left:12px;"></div>
                        <div class="spot" style="width:20px; height:20px; top:40px; left:22px;"></div>
                        <div class="spot" style="width:16px; height:16px; top:75px; left:10px;"></div>
                    </div>
                    <div class="wing wing-right">
                        <div class="spot" style="width:14px; height:14px; top:15px; right:12px;"></div>
                        <div class="spot" style="width:20px; height:20px; top:40px; right:22px;"></div>
                        <div class="spot" style="width:16px; height:16px; top:75px; right:10px;"></div>
                    </div>
                </div>
                
                <div class="timeline">
                    <div class="pill"><span>Uptime</span> <strong id="val-uptime">--</strong></div>
                    <div class="pill"><span>Optimum Streak</span> <strong id="val-streak" style="color:#22CC44;">--</strong></div>
                    <div class="pill"><span>Last Watered</span> <strong id="val-lastwater">--</strong></div>
                </div>
            </div>

            <div class="glass-panel" style="margin-bottom: 20px;">
                <div style="display:flex; justify-content:space-between; align-items:center; margin-bottom: 15px;">
                    <div class="section-title" style="margin:0;">Plant Library</div>
                    <select id="plant-selector" class="glass-select" onchange="changePlant()">
                        <option value="basil">🌿 Basil Berry</option>
                        <option value="monstera">🍃 Monstera Deliciosa</option>
                        <option value="cactus">🌵 Saguaro Cactus</option>
                    </select>
                </div>
                <div>
                    <h3 id="plant-name" style="margin: 0 0 10px 0; font-size: 1.2rem;">Basil (Ocimum basilicum)</h3>
                    <p id="plant-desc" style="font-size: 0.9rem; color: var(--text-dim); line-height: 1.5; margin: 0;">
                        Thrives in warm, sunny environments (20-25°C). Prefers well-drained soil and consistent moisture. Pinch off early flowers to encourage leaf growth.
                    </p>
                    <div class="alarm-card" id="plant-alarms">
                        <strong>Active Hardware Alarms:</strong><br><br>
                        💧 <strong>Thirsty:</strong> Triggers a stuttering sweep if soil drops below <strong>30%</strong>.<br>
                        🔥 <strong>Heatwave:</strong> Triggers a frantic sweep if ambient temperature exceeds <strong>28.0°C</strong>.
                    </div>
                </div>
            </div>

            <div class="controls">
                <button class="btn" onclick="triggerWings('normal')">🦋 Normal Sweep</button>
                <button class="btn btn-water" onclick="triggerWings('water')">💧 Sim Low Water</button>
                <button class="btn btn-temp" onclick="triggerWings('temp')">🔥 Sim High Temp</button>
            </div>
        </aside>

        <main>
            <div class="glass-panel" style="margin-bottom: 20px; display: flex; justify-content: center; align-items: center; overflow: hidden; padding: 15px;">
                <div id="weatherapi-weather-widget-3"></div><script type='text/javascript' src='https://www.weatherapi.com/weather/widget.ashx?loc=3328576&wid=3&tu=1&div=weatherapi-weather-widget-3' async></script><noscript><a href="https://www.weatherapi.com/weather/q/eindhoven-3328576" alt="Hour by hour Eindhoven weather">10 day hour by hour Eindhoven weather</a></noscript>
            </div>

            <div class="glass-panel" style="margin-bottom: 20px;">
                <div class="section-title">Current Telemetry</div>
                <div class="sensor-grid">
                    <div class="metric">
                        <p>Soil Moisture</p>
                        <h2 id="soil-val">%SOIL%%</h2>
                    </div>
                    <div class="metric">
                        <p>Temperature</p>
                        <h2 id="temp-val">%TEMP% °C</h2>
                    </div>
                    <div class="metric">
                        <p>Humidity</p>
                        <h2 id="hum-val">%HUM%%</h2>
                    </div>
                </div>
            </div>

            <div class="glass-panel" style="margin-bottom: 20px;">
                <div class="section-title">Routine Care Tasks</div>
                <div class="task-list">
                    <label class="task-item">
                        <input type="checkbox">
                        <span class="checkmark"></span>
                        <span class="task-text">💧 Mist Leaves</span>
                    </label>
                    <label class="task-item">
                        <input type="checkbox">
                        <span class="checkmark"></span>
                        <span class="task-text">✂️ Prune Dead Growth</span>
                    </label>
                    <label class="task-item">
                        <input type="checkbox">
                        <span class="checkmark"></span>
                        <span class="task-text">🧽 Clean Leaves</span>
                    </label>
                    <label class="task-item">
                        <input type="checkbox">
                        <span class="checkmark"></span>
                        <span class="task-text">🪴 Check Soil & Repot</span>
                    </label>
                </div>
            </div>

            <div class="glass-panel">
                <div class="section-title">7-Day Moisture Trend</div>
                <div class="chart-container">
                    <canvas id="moistureChart"></canvas>
                </div>
            </div>
        </main>
    </div>

    <script>
        // --- PLANT LIBRARY DATABASE ---
        const plantLibrary = {
            basil: {
                name: "Basil (Ocimum basilicum)",
                desc: "Thrives in warm, sunny environments (20-25°C). Prefers well-drained soil and consistent moisture. Pinch off early flowers to encourage leaf growth.",
                alarms: "<strong>Active Hardware Alarms:</strong><br><br>💧 <strong>Thirsty:</strong> Triggers a stuttering sweep if soil drops below <strong>30%</strong>.<br>🔥 <strong>Heatwave:</strong> Triggers a frantic sweep if ambient temp exceeds <strong>28.0°C</strong>."
            },
            monstera: {
                name: "Monstera (Monstera deliciosa)",
                desc: "Prefers bright, indirect light and high humidity (60%+). Allow the top few inches of soil to dry out between waterings. Prone to root rot if overwatered.",
                alarms: "<strong>Target Profile Loaded:</strong><br><br>💧 <strong>Thirsty:</strong> Triggers a stuttering sweep if soil drops below <strong>15%</strong>.<br>🔥 <strong>Heatwave:</strong> Triggers a frantic sweep if ambient temp exceeds <strong>30.0°C</strong>."
            },
            cactus: {
                name: "Saguaro Cactus (Carnegiea gigantea)",
                desc: "Requires full sun and extremely well-draining soil. Only water when the soil is completely bone dry. Highly drought and heat resistant.",
                alarms: "<strong>Target Profile Loaded:</strong><br><br>💧 <strong>Thirsty:</strong> Triggers a stuttering sweep if soil drops below <strong>5%</strong>.<br>🔥 <strong>Heatwave:</strong> Triggers a frantic sweep if ambient temp exceeds <strong>35.0°C</strong>."
            }
        };

        function changePlant() {
            const selected = document.getElementById('plant-selector').value;
            const plant = plantLibrary[selected];
            document.getElementById('plant-name').innerText = plant.name;
            document.getElementById('plant-desc').innerText = plant.desc;
            document.getElementById('plant-alarms').innerHTML = plant.alarms;
        }

        // --- CHART & DATA HANDLING ---
        const ctx = document.getElementById('moistureChart').getContext('2d');
        let historicalData = [45, 42, 38, 35, 30, 28, %SOIL%]; 
        
        const moistureChart = new Chart(ctx, {
            type: 'line',
            data: {
                labels: ['Day -6', 'Day -5', 'Day -4', 'Day -3', 'Day -2', 'Yesterday', 'Now'],
                datasets: [{
                    label: 'Soil Moisture %',
                    data: historicalData,
                    borderColor: '#0088FF',
                    backgroundColor: 'rgba(0, 136, 255, 0.1)',
                    borderWidth: 3,
                    tension: 0.4,
                    fill: true,
                    pointBackgroundColor: '#ffffff',
                    pointBorderColor: '#0088FF',
                    pointRadius: 4
                }]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false, 
                plugins: { legend: { display: false } },
                scales: {
                    y: { 
                        beginAtZero: true, max: 100, 
                        grid: { color: 'rgba(255,255,255,0.05)' },
                        ticks: { color: '#999' }
                    },
                    x: { 
                        grid: { display: false },
                        ticks: { color: '#999' }
                    }
                }
            }
        });

        function updateAmbientColor(temp, moisture) {
            let h = Math.max(0, Math.min(220, 220 - ((temp - 18) * 15))); 
            let s = Math.max(30, Math.min(100, 100 - moisture + 20));
            let l = 15; 
            
            const ambient = document.getElementById('ambient-bg');
            ambient.style.background = `radial-gradient(circle at 50% 0%, hsla(${h}, ${s}%, ${l}%, 0.8), #050505 70%)`;
        }

        setInterval(() => {
            fetch('/api/data')
                .then(res => res.json())
                .then(data => {
                    document.getElementById('soil-val').innerText = data.soil_moisture + '%';
                    document.getElementById('temp-val').innerText = data.temperature.toFixed(1) + ' °C';
                    document.getElementById('hum-val').innerText = data.humidity + '%';
                    
                    document.getElementById('val-uptime').innerText = data.uptime_mins + "m";
                    document.getElementById('val-lastwater').innerText = (data.last_watered_mins >= 0) ? data.last_watered_mins + "m ago" : "Never";
                    document.getElementById('val-streak').innerText = data.streak_hours + "h";

                    document.getElementById('net-status').style.background = '#22CC44';
                    document.getElementById('net-status').style.boxShadow = '0 0 10px #22CC44';

                    updateAmbientColor(data.temperature, data.soil_moisture);

                    moistureChart.data.datasets[0].data[6] = data.soil_moisture;
                    moistureChart.update();
                })
                .catch(err => {
                    document.getElementById('net-status').style.background = '#FF4444';
                    document.getElementById('net-status').style.boxShadow = '0 0 10px #FF4444';
                });
        }, 3000);

        function triggerWings(mode) {
            fetch('/flap?mode=' + mode);
            const bug = document.getElementById('ladybug');
            bug.className = 'bug-container flap-' + mode; 
            setTimeout(() => { bug.className = 'bug-container'; }, 4000); 
        }
        
        updateAmbientColor(%TEMP%, %SOIL%);
    </script>
</body>
</html>
)rawliteral";

String getDashboardHTML(int soil, float temp, float hum) {
    String html = dashboard_html_template;
    html.replace("%SOIL%", String(soil));
    html.replace("%TEMP%", String(temp, 1));
    html.replace("%HUM%", String(hum, 0));
    return html;
}

#endif