let chart;
let sensorData = [];
const MAX_DATA_POINTS = 100;
let lastFetchTime = 0;
const FETCH_INTERVAL = 50; // 50ms interval

function initChart() {
    chart = Chart('#sensorChart', {
        lines: [{
            data: sensorData
        }],
        xAxis: {
            title: 'Time',
            min: 0,
            max: MAX_DATA_POINTS,
            ticks: 10
        },
        yAxis: {
            title: 'Sensor Value',
            min: 0,
            max: 1000, // Adjust based on your sensor's range
            ticks: 100
        }
    });
}

function fetchSensorData() {
    const now = performance.now();
    if (now - lastFetchTime < FETCH_INTERVAL) {
        requestAnimationFrame(fetchSensorData);
        return;
    }
    lastFetchTime = now;

    fetch('/api/sensor-data')
        .then(response => response.text())
        .then(data => {
            updateChart(parseInt(data));
            requestAnimationFrame(fetchSensorData);
        })
        .catch(error => {
            console.error('Error fetching sensor data:', error);
            requestAnimationFrame(fetchSensorData);
        });
}

function updateChart(value) {
    sensorData.push([sensorData.length, value]);
    if (sensorData.length > MAX_DATA_POINTS) {
        sensorData.shift();
        sensorData = sensorData.map((point, index) => [index, point[1]]);
    }

    initChart();
}

function loadConfig() {
    fetch('/api/config')
        .then(response => response.json())
        .then(config => {
            document.getElementById('sensor_name').value = config.sensorName || '';
            document.getElementById('mqtt_server').value = config.mqttBroker || '';
            document.getElementById('mqtt_port').value = config.mqttPort || '';
            document.getElementById('mqtt_topic').value = config.mqttTopic || '';
        })
        .catch(error => console.error('Error loading config:', error));
}

function saveConfig(event) {
    event.preventDefault();
    const config = {
        sensorName: document.getElementById('sensor_name').value,
        mqttBroker: document.getElementById('mqtt_server').value,
        mqttPort: document.getElementById('mqtt_port').value,
        mqttTopic: document.getElementById('mqtt_topic').value
    };

    fetch('/api/config', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/json',
        },
        body: JSON.stringify(config),
    })
    .then(response => response.text())
    .then(result => {
        alert(result);
    })
    .catch(error => console.error('Error saving config:', error));
}

function handleOTAUpdate(event) {
    event.preventDefault();
    const formData = new FormData(event.target);

    fetch('/api/ota', {
        method: 'POST',
        body: formData,
    })
    .then(response => response.text())
    .then(result => {
        alert(result);
        if (result === 'Update success.') {
            setTimeout(() => location.reload(), 5000);
        }
    })
    .catch(error => console.error('Error during OTA update:', error));
}

// Initialize
document.addEventListener('DOMContentLoaded', function() {
    loadConfig();
    initChart();
    requestAnimationFrame(fetchSensorData);

    // Add event listeners
    document.getElementById('configForm').addEventListener('submit', saveConfig);
    document.getElementById('otaForm').addEventListener('submit', handleOTAUpdate);
});