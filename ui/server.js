/**
 * @file server.js
 * @brief Express + WebSocket Server for VEX V5 Brain UI
 */

const express = require('express');
const WebSocket = require('ws');
const path = require('path');

const HTTP_PORT = process.env.HTTP_PORT || 3000;
const WS_PORT = process.env.WS_PORT || 9000;

// Create Express app
const app = express();

// Serve static files from the ui directory
app.use(express.static(__dirname));

// Health check endpoint
app.get('/health', (req, res) => {
    res.json({ status: 'ok', wsPort: WS_PORT });
});

// Start HTTP server
const httpServer = app.listen(HTTP_PORT, () => {
    console.log(`HTTP server running at http://localhost:${HTTP_PORT}`);
    console.log(`Open http://localhost:${HTTP_PORT} in your browser`);
});

// Create WebSocket server
const wss = new WebSocket.Server({ port: WS_PORT });

// Connected clients
let hostClient = null;
const uiClients = new Set();

// Handle WebSocket connections
wss.on('connection', (ws, req) => {
    const clientType = req.url === '/host' ? 'host' : 'ui';
    
    if (clientType === 'host') {
        if (hostClient) {
            console.log('Warning: Host already connected, replacing...');
            hostClient.close();
        }
        hostClient = ws;
        console.log('Host application connected');
        
        // Notify UI clients
        broadcastToUI({ type: 'host_status', connected: true });
    } else {
        uiClients.add(ws);
        console.log(`UI client connected (${uiClients.size} total)`);
        
        // Send current host status
        ws.send(JSON.stringify({ type: 'host_status', connected: hostClient !== null }));
    }
    
    ws.on('message', (data) => {
        try {
            const message = JSON.parse(data.toString());
            handleMessage(ws, clientType, message);
        } catch (e) {
            console.error('Error parsing message:', e);
        }
    });
    
    ws.on('close', () => {
        if (clientType === 'host') {
            hostClient = null;
            console.log('Host application disconnected');
            broadcastToUI({ type: 'host_status', connected: false });
        } else {
            uiClients.delete(ws);
            console.log(`UI client disconnected (${uiClients.size} remaining)`);
        }
    });
    
    ws.on('error', (err) => {
        console.error(`WebSocket error (${clientType}):`, err.message);
    });
});

console.log(`WebSocket server running on port ${WS_PORT}`);

// Broadcast to all UI clients
function broadcastToUI(message) {
    const data = JSON.stringify(message);
    uiClients.forEach((client) => {
        if (client.readyState === WebSocket.OPEN) {
            client.send(data);
        }
    });
}

// Send to host
function sendToHost(message) {
    if (hostClient && hostClient.readyState === WebSocket.OPEN) {
        hostClient.send(JSON.stringify(message));
    }
}

// Handle incoming messages
function handleMessage(ws, clientType, message) {
    if (clientType === 'host') {
        // Messages from host -> broadcast to UI
        switch (message.type) {
            case 'screen':
                // Forward screen updates to UI
                broadcastToUI(message);
                break;
                
            case 'motor':
                // Forward motor telemetry to UI
                broadcastToUI(message);
                break;
                
            case 'log':
                // Forward log messages to UI
                console.log(`[${message.level}] ${message.msg}`);
                broadcastToUI(message);
                break;
                
            case 'autons':
                // Forward autonomous list to UI
                broadcastToUI(message);
                break;
                
            case 'lcd':
                // Forward LCD updates to UI
                broadcastToUI(message);
                break;
                
            case 'mode':
                // Forward current mode to UI
                broadcastToUI(message);
                break;
                
            default:
                // Forward unknown messages
                broadcastToUI(message);
        }
    } else {
        // Messages from UI -> send to host
        switch (message.type) {
            case 'touch':
                // Forward touch input to host
                sendToHost(message);
                break;
                
            case 'controller':
                // Forward controller input to host
                sendToHost(message);
                break;
                
            case 'mode':
                // Forward mode change to host
                console.log(`Mode change requested: ${message.value}`);
                sendToHost(message);
                broadcastToUI({ type: 'mode', value: message.value });
                break;
                
            case 'select_auto':
                // Forward auto selection to host
                console.log(`Auto selected: ${message.category} #${message.index}`);
                sendToHost(message);
                break;
                
            default:
                // Forward unknown messages to host
                sendToHost(message);
        }
    }
}

// Graceful shutdown
process.on('SIGINT', () => {
    console.log('\nShutting down...');
    
    // Close all WebSocket connections
    if (hostClient) hostClient.close();
    uiClients.forEach((client) => client.close());
    
    wss.close(() => {
        httpServer.close(() => {
            console.log('Server shut down');
            process.exit(0);
        });
    });
});
