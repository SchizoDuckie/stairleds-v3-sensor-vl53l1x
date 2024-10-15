class ConfigManager {
    async saveConfig(config) {
        try {
            const response = await fetch('/api/config', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(config)
            });
            const data = await response.json();
            if (data.success) {
                return data;
            } else {
                throw new Error('Failed to save configuration');
            }
        } catch (error) {
            console.error('Error saving config:', error);
            throw error;
        }
    }

    async loadConfig() {
        try {
            const response = await fetch('/api/config');
            return await response.json();
        } catch (error) {
            console.error('Error loading config:', error);
            throw error;
        }
    }
}
