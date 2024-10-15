class MQTTManager {
    constructor(configManager) {
        this.configManager = configManager;
        this.mqttConfig = document.getElementById('mqttConfig');
        this.toggleMqttConfigButton = document.getElementById('toggleMqttConfig');
        this.saveMqttConfigButton = document.getElementById('saveMqttConfig');
        this.mqttBrokerInput = document.getElementById('mqttBroker');
        this.mqttPortInput = document.getElementById('mqttPort');
        this.mqttTopicInput = document.getElementById('mqttTopic');

        this.bindEvents();
    }

    bindEvents() {
        this.toggleMqttConfigButton.addEventListener('click', () => this.toggleMqttConfig());
        this.saveMqttConfigButton.addEventListener('click', () => this.saveMqttConfig());
    }

    toggleMqttConfig() {
        this.mqttConfig.hidden = !this.mqttConfig.hidden;
        this.toggleMqttConfigButton.textContent = 
            this.mqttConfig.hidden ? 'Show MQTT Settings' : 'Hide MQTT Settings';
    }

    async saveMqttConfig() {
        const config = {
            mqttBroker: this.mqttBrokerInput.value.trim(),
            mqttPort: this.mqttPortInput.value,
            mqttTopic: this.mqttTopicInput.value.trim()
        };
        if (!config.mqttBroker || !config.mqttPort || !config.mqttTopic) {
            alert('Please fill in all MQTT fields.');
            return;
        }
        try {
            await this.configManager.saveConfig(config);
            alert('MQTT configuration saved successfully.');
        } catch (error) {
            alert('Failed to save MQTT configuration. Please try again.');
            console.error(error);
        }
    }

    loadMqttConfig(config) {
        this.mqttBrokerInput.value = config.mqttBroker || '';
        this.mqttPortInput.value = config.mqttPort || '';
        this.mqttTopicInput.value = config.mqttTopic || '';
    }
}
