/**
 * @file app.js
 * @brief Client-side JavaScript for VEX V5 Brain UI
 */

// WebSocket connection
let ws = null;
let connected = false;

// State
let currentMode = 'disabled';
let selectedMatchAuto = -1;
let selectedSkillsAuto = -1;
let matchAutos = [];
let skillsAutos = [];

// Joystick state
const joystickState = {
    left: { x: 0, y: 0, active: false },
    right: { x: 0, y: 0, active: false }
};

// Button state
const buttonState = {
    a: false, b: false, x: false, y: false,
    up: false, down: false, left: false, right: false,
    l1: false, l2: false, r1: false, r2: false
};

// Initialize on page load
document.addEventListener('DOMContentLoaded', () => {
    initWebSocket();
    initBrainScreen();
    initModeButtons();
    initController();
    initAutonTabs();
    initMotorGrid();
    
    // Reconnect on visibility change
    document.addEventListener('visibilitychange', () => {
        if (document.visibilityState === 'visible' && !connected) {
            initWebSocket();
        }
    });
});

// WebSocket connection
function initWebSocket() {
    const wsUrl = `ws://localhost:9000`;
    
    try {
        ws = new WebSocket(wsUrl);
        
        ws.onopen = () => {
            connected = true;
            updateConnectionStatus(true);
            log('info', 'Connected to server');
        };
        
        ws.onclose = () => {
            connected = false;
            updateConnectionStatus(false);
            log('warn', 'Disconnected from server');
            
            // Attempt reconnect after 3 seconds
            setTimeout(initWebSocket, 3000);
        };
        
        ws.onerror = (err) => {
            log('error', 'WebSocket error');
        };
        
        ws.onmessage = (event) => {
            try {
                const message = JSON.parse(event.data);
                handleMessage(message);
            } catch (e) {
                console.error('Error parsing message:', e);
            }
        };
    } catch (e) {
        console.error('WebSocket connection failed:', e);
        setTimeout(initWebSocket, 3000);
    }
}

function send(message) {
    if (ws && ws.readyState === WebSocket.OPEN) {
        ws.send(JSON.stringify(message));
    }
}

function updateConnectionStatus(isConnected) {
    const dot = document.getElementById('status-dot');
    const text = document.getElementById('status-text');
    
    if (isConnected) {
        dot.classList.remove('disconnected');
        dot.classList.add('connected');
        text.textContent = 'Connected';
    } else {
        dot.classList.remove('connected');
        dot.classList.add('disconnected');
        text.textContent = 'Disconnected';
    }
}

// Handle incoming messages
function handleMessage(message) {
    switch (message.type) {
        case 'host_status':
            if (message.connected) {
                log('info', 'Host application connected');
            } else {
                log('warn', 'Host application disconnected');
            }
            break;
            
        case 'screen':
            updateScreen(message);
            break;
            
        case 'motor':
            updateMotor(message);
            break;
            
        case 'log':
            log(message.level, message.msg);
            break;
            
        case 'autons':
            updateAutons(message);
            break;
            
        case 'lcd':
            updateLCD(message);
            break;
            
        case 'mode':
            setModeUI(message.value);
            break;
    }
}

// Brain screen
let screenCtx = null;

function initBrainScreen() {
    const canvas = document.getElementById('brain-screen');
    screenCtx = canvas.getContext('2d');
    
    // Fill with black initially
    screenCtx.fillStyle = '#000';
    screenCtx.fillRect(0, 0, 480, 272);
    
    // Touch handling
    canvas.addEventListener('mousedown', (e) => handleTouch(e, true));
    canvas.addEventListener('mouseup', (e) => handleTouch(e, false));
    canvas.addEventListener('mousemove', (e) => {
        if (e.buttons === 1) handleTouch(e, true);
    });
    canvas.addEventListener('mouseleave', (e) => handleTouch(e, false));
    
    // Touch events for mobile
    canvas.addEventListener('touchstart', (e) => {
        e.preventDefault();
        handleTouch(e.touches[0], true, canvas);
    });
    canvas.addEventListener('touchend', (e) => {
        e.preventDefault();
        handleTouch(e.changedTouches[0], false, canvas);
    });
    canvas.addEventListener('touchmove', (e) => {
        e.preventDefault();
        handleTouch(e.touches[0], true, canvas);
    });
    
    // LCD buttons
    document.getElementById('lcd-btn-left').addEventListener('mousedown', () => sendLCDButton('left', true));
    document.getElementById('lcd-btn-left').addEventListener('mouseup', () => sendLCDButton('left', false));
    document.getElementById('lcd-btn-center').addEventListener('mousedown', () => sendLCDButton('center', true));
    document.getElementById('lcd-btn-center').addEventListener('mouseup', () => sendLCDButton('center', false));
    document.getElementById('lcd-btn-right').addEventListener('mousedown', () => sendLCDButton('right', true));
    document.getElementById('lcd-btn-right').addEventListener('mouseup', () => sendLCDButton('right', false));
}

function handleTouch(e, pressed, canvas = null) {
    const target = canvas || e.target;
    const rect = target.getBoundingClientRect();
    const x = Math.round((e.clientX - rect.left) * (480 / rect.width));
    const y = Math.round((e.clientY - rect.top) * (272 / rect.height));
    
    send({ type: 'touch', x, y, pressed });
}

function sendLCDButton(button, pressed) {
    let btnValue = 0;
    switch (button) {
        case 'left': btnValue = 4; break;
        case 'center': btnValue = 2; break;
        case 'right': btnValue = 1; break;
    }
    send({ type: 'lcd_button', button: btnValue, pressed });
}

function updateScreen(data) {
    if (!screenCtx) return;
    
    // Decode base64 data
    const binaryString = atob(data.data);
    const bytes = new Uint8Array(binaryString.length);
    for (let i = 0; i < binaryString.length; i++) {
        bytes[i] = binaryString.charCodeAt(i);
    }
    
    // Create image data
    const width = data.x2 - data.x1 + 1;
    const height = data.y2 - data.y1 + 1;
    const imageData = screenCtx.createImageData(width, height);
    
    // Convert RGB565 to RGBA
    for (let i = 0, j = 0; i < bytes.length; i += 2, j += 4) {
        const pixel = bytes[i] | (bytes[i + 1] << 8);
        const r = ((pixel >> 11) & 0x1F) << 3;
        const g = ((pixel >> 5) & 0x3F) << 2;
        const b = (pixel & 0x1F) << 3;
        
        imageData.data[j] = r;
        imageData.data[j + 1] = g;
        imageData.data[j + 2] = b;
        imageData.data[j + 3] = 255;
    }
    
    screenCtx.putImageData(imageData, data.x1, data.y1);
}

function updateLCD(data) {
    // For LLEMU-style display, we draw text on the canvas
    if (!screenCtx || !data.lines) return;
    
    screenCtx.fillStyle = '#000';
    screenCtx.fillRect(0, 0, 480, 272);
    
    screenCtx.fillStyle = '#0f0';
    screenCtx.font = '16px monospace';
    
    data.lines.forEach((line, i) => {
        screenCtx.fillText(line, 10, 30 + i * 24);
    });
}

// Mode buttons
function initModeButtons() {
    const buttons = {
        'mode-disabled': 'disabled',
        'mode-autonomous': 'autonomous',
        'mode-opcontrol': 'opcontrol'
    };
    
    Object.entries(buttons).forEach(([id, mode]) => {
        document.getElementById(id).addEventListener('click', () => {
            setMode(mode);
        });
    });
}

function setMode(mode) {
    send({ type: 'mode', value: mode });
    setModeUI(mode);
}

function setModeUI(mode) {
    currentMode = mode;
    
    document.querySelectorAll('.mode-btn').forEach(btn => {
        btn.classList.remove('active');
    });
    
    const modeMap = {
        'disabled': 'mode-disabled',
        'autonomous': 'mode-autonomous',
        'opcontrol': 'mode-opcontrol'
    };
    
    const btn = document.getElementById(modeMap[mode]);
    if (btn) btn.classList.add('active');
}

// Controller
function initController() {
    initJoystick('joystick-left', 'left');
    initJoystick('joystick-right', 'right');
    initControllerButtons();
}

function initJoystick(canvasId, side) {
    const canvas = document.getElementById(canvasId);
    const ctx = canvas.getContext('2d');
    const centerX = 60, centerY = 60, radius = 50;
    
    function draw() {
        ctx.clearRect(0, 0, 120, 120);
        
        // Background
        ctx.fillStyle = '#333';
        ctx.beginPath();
        ctx.arc(centerX, centerY, radius, 0, Math.PI * 2);
        ctx.fill();
        
        // Center
        ctx.fillStyle = '#555';
        ctx.beginPath();
        ctx.arc(centerX, centerY, 10, 0, Math.PI * 2);
        ctx.fill();
        
        // Stick position
        const state = joystickState[side];
        const stickX = centerX + (state.x / 127) * (radius - 15);
        const stickY = centerY - (state.y / 127) * (radius - 15);
        
        ctx.fillStyle = state.active ? '#e94560' : '#888';
        ctx.beginPath();
        ctx.arc(stickX, stickY, 15, 0, Math.PI * 2);
        ctx.fill();
    }
    
    draw();
    
    function updateJoystick(e, active) {
        const rect = canvas.getBoundingClientRect();
        const x = e.clientX - rect.left - centerX;
        const y = -(e.clientY - rect.top - centerY);
        
        // Clamp to radius
        const dist = Math.sqrt(x * x + y * y);
        const maxDist = radius - 15;
        const scale = dist > maxDist ? maxDist / dist : 1;
        
        joystickState[side].x = Math.round((x * scale / maxDist) * 127);
        joystickState[side].y = Math.round((y * scale / maxDist) * 127);
        joystickState[side].active = active;
        
        draw();
        sendControllerState();
    }
    
    canvas.addEventListener('mousedown', (e) => updateJoystick(e, true));
    canvas.addEventListener('mousemove', (e) => {
        if (joystickState[side].active) updateJoystick(e, true);
    });
    canvas.addEventListener('mouseup', () => {
        joystickState[side] = { x: 0, y: 0, active: false };
        draw();
        sendControllerState();
    });
    canvas.addEventListener('mouseleave', () => {
        if (joystickState[side].active) {
            joystickState[side] = { x: 0, y: 0, active: false };
            draw();
            sendControllerState();
        }
    });
}

function initControllerButtons() {
    const buttonMap = {
        'btn-a': 'a', 'btn-b': 'b', 'btn-x': 'x', 'btn-y': 'y',
        'btn-up': 'up', 'btn-down': 'down', 'btn-left': 'left', 'btn-right': 'right',
        'btn-l1': 'l1', 'btn-l2': 'l2', 'btn-r1': 'r1', 'btn-r2': 'r2'
    };
    
    Object.entries(buttonMap).forEach(([id, key]) => {
        const btn = document.getElementById(id);
        
        btn.addEventListener('mousedown', () => {
            buttonState[key] = true;
            btn.classList.add('pressed');
            sendControllerState();
        });
        
        btn.addEventListener('mouseup', () => {
            buttonState[key] = false;
            btn.classList.remove('pressed');
            sendControllerState();
        });
        
        btn.addEventListener('mouseleave', () => {
            if (buttonState[key]) {
                buttonState[key] = false;
                btn.classList.remove('pressed');
                sendControllerState();
            }
        });
    });
    
    // Keyboard support
    document.addEventListener('keydown', (e) => {
        const key = keyToButton(e.key);
        if (key && !e.repeat) {
            buttonState[key] = true;
            const btn = document.getElementById('btn-' + key);
            if (btn) btn.classList.add('pressed');
            sendControllerState();
        }
    });
    
    document.addEventListener('keyup', (e) => {
        const key = keyToButton(e.key);
        if (key) {
            buttonState[key] = false;
            const btn = document.getElementById('btn-' + key);
            if (btn) btn.classList.remove('pressed');
            sendControllerState();
        }
    });
}

function keyToButton(key) {
    const map = {
        'a': 'a', 'b': 'b', 'x': 'x', 'y': 'y',
        'ArrowUp': 'up', 'ArrowDown': 'down', 'ArrowLeft': 'left', 'ArrowRight': 'right',
        'q': 'l1', 'w': 'l2', 'e': 'r1', 'r': 'r2'
    };
    return map[key];
}

function sendControllerState() {
    let buttons = 0;
    if (buttonState.a) buttons |= 0x01;
    if (buttonState.b) buttons |= 0x02;
    if (buttonState.x) buttons |= 0x04;
    if (buttonState.y) buttons |= 0x08;
    if (buttonState.up) buttons |= 0x10;
    if (buttonState.down) buttons |= 0x20;
    if (buttonState.left) buttons |= 0x40;
    if (buttonState.right) buttons |= 0x80;
    if (buttonState.l1) buttons |= 0x100;
    if (buttonState.l2) buttons |= 0x200;
    if (buttonState.r1) buttons |= 0x400;
    if (buttonState.r2) buttons |= 0x800;
    
    send({
        type: 'controller',
        analog: {
            lx: joystickState.left.x,
            ly: joystickState.left.y,
            rx: joystickState.right.x,
            ry: joystickState.right.y
        },
        digital: buttons
    });
}

// Autonomous selector
function initAutonTabs() {
    document.getElementById('tab-match').addEventListener('click', () => {
        document.getElementById('tab-match').classList.add('active');
        document.getElementById('tab-skills').classList.remove('active');
        document.getElementById('match-autos').classList.remove('hidden');
        document.getElementById('skills-autos').classList.add('hidden');
    });
    
    document.getElementById('tab-skills').addEventListener('click', () => {
        document.getElementById('tab-skills').classList.add('active');
        document.getElementById('tab-match').classList.remove('active');
        document.getElementById('skills-autos').classList.remove('hidden');
        document.getElementById('match-autos').classList.add('hidden');
    });
}

function updateAutons(data) {
    matchAutos = data.match || [];
    skillsAutos = data.skills || [];
    
    renderAutonList('match-autos', matchAutos, 'match');
    renderAutonList('skills-autos', skillsAutos, 'skills');
}

function renderAutonList(containerId, autos, category) {
    const container = document.getElementById(containerId);
    container.innerHTML = '';
    
    if (autos.length === 0) {
        container.innerHTML = '<div class="auton-item"><span class="name">No autonomous routines registered</span></div>';
        return;
    }
    
    autos.forEach((auto, index) => {
        const item = document.createElement('div');
        item.className = 'auton-item';
        if ((category === 'match' && index === selectedMatchAuto) ||
            (category === 'skills' && index === selectedSkillsAuto)) {
            item.classList.add('selected');
        }
        
        item.innerHTML = `
            <div class="name">${auto.name}</div>
            <div class="desc">${auto.desc || ''}</div>
        `;
        
        item.addEventListener('click', () => {
            selectAuto(category, index);
        });
        
        container.appendChild(item);
    });
}

function selectAuto(category, index) {
    if (category === 'match') {
        selectedMatchAuto = index;
    } else {
        selectedSkillsAuto = index;
    }
    
    send({ type: 'select_auto', category, index });
    
    // Update UI
    const containerId = category === 'match' ? 'match-autos' : 'skills-autos';
    const container = document.getElementById(containerId);
    container.querySelectorAll('.auton-item').forEach((item, i) => {
        item.classList.toggle('selected', i === index);
    });
}

// Motor telemetry
function initMotorGrid() {
    const grid = document.getElementById('motor-grid');
    
    for (let i = 1; i <= 8; i++) {
        const slot = document.createElement('div');
        slot.className = 'motor-slot';
        slot.id = `motor-${i}`;
        slot.innerHTML = `
            <div class="port">Port ${i}</div>
            <div class="value">--</div>
            <div class="bar"><div class="bar-fill" style="width: 50%"></div></div>
        `;
        grid.appendChild(slot);
    }
}

function updateMotor(data) {
    const slot = document.getElementById(`motor-${data.port}`);
    if (!slot) return;
    
    const value = slot.querySelector('.value');
    const barFill = slot.querySelector('.bar-fill');
    
    value.textContent = `${data.voltage}V | ${Math.round(data.velocity)}RPM`;
    
    const percent = Math.abs(data.voltage / 127) * 100;
    barFill.style.width = `${percent}%`;
    barFill.style.backgroundColor = data.voltage >= 0 ? '#4caf50' : '#f44336';
}

// Console logging
function log(level, message) {
    const console = document.getElementById('console');
    const line = document.createElement('div');
    line.className = `console-line ${level}`;
    
    const time = new Date().toLocaleTimeString();
    line.textContent = `[${time}] ${message}`;
    
    console.appendChild(line);
    console.scrollTop = console.scrollHeight;
    
    // Limit console lines
    while (console.children.length > 100) {
        console.removeChild(console.firstChild);
    }
}
