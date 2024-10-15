document.addEventListener('DOMContentLoaded', function() {
    const sensorData = new SensorData();
    const chartManager = new ChartManager(sensorData);
    const configManager = new ConfigManager();
    const setupManager = new SetupManager(configManager, sensorData);
    const mqttManager = new MQTTManager(configManager);
    const wifiManager = new WifiManager();

    console.log('All managers initialized');

    chartManager.initChart();

    async function updateLoop() {
        await sensorData.fetch();
        chartManager.updateChart();
        requestAnimationFrame(updateLoop);
    }

    updateLoop();

    // Initial status check and periodic updates
    setupManager.checkConnectionStatus();
    setInterval(() => setupManager.checkConnectionStatus(), 10000);

    // Load initial config
    configManager.loadConfig().then(config => {
        console.log('Initial config loaded:', config);
        document.getElementById('sensorName').value = config.sensorName || '';
        mqttManager.loadMqttConfig(config);
        setupManager.checkConnectionStatus();  // Recheck status after loading config
    });
});