class SetupManager {
    constructor(configManager, sensorData) {
        this.configManager = configManager;
        this.sensorData = sensorData;
        this.currentStep = 1;
        this.statusElement = document.getElementById('connectionStatus');
        this.setupSteps = document.querySelectorAll('.setup-step');
        this.sensorNameInput = document.getElementById('sensorName');
        this.sensorNameFeedback = document.getElementById('sensorNameFeedback');
        this.setSensorNameButton = document.getElementById('setSensorName');
        this.sensorNameStatus = document.getElementById('sensorNameStatus');
        this.wifiStatus = document.getElementById('wifiStatus');
        this.serverStatus = document.getElementById('serverStatus');
        this.sensorNameIcon = document.getElementById('sensorNameIcon');
        this.wifiIcon = document.getElementById('wifiIcon');
        this.serverIcon = document.getElementById('serverIcon');

        this.bindEvents();
        this.showAllSteps();
        
    }

    bindEvents() {
        this.setSensorNameButton.addEventListener('click', () => this.setSensorName());
        this.sensorNameInput.addEventListener('input', () => this.updateSensorNameFeedback());

    }

    showAllSteps() {
        this.setupSteps.forEach(step => {
            step.hidden = false;
        });
    }

    async checkConnectionStatus() {
        try {
            const response = await fetch('/api/sensor-status');
            const status = await response.json();
            console.log('Sensor status:', status);
            this.updateStatusDisplay(status);
            this.updateStepVisibility(status);
        } catch (error) {
            console.error('Error checking sensor status:', error);
            this.displayError('Error checking connection status');
        }
    }

    updateStatusDisplay(status) {
        // Update sensor name status
        this.sensorNameStatus.textContent = status.sensorName || 'Not set';
        this.sensorNameIcon.textContent = status.sensorName ? '✅' : '❌';

        // Update WiFi status
        if (status.connected) {
            this.wifiStatus.textContent = status.ssid;
            this.wifiIcon.textContent = '✅';
            this.statusElement.textContent = `Connected to ${status.ssid} (IP: ${status.ip})`;
            this.statusElement.className = 'connected';
        } else if (status.isAPMode) {
            this.wifiStatus.textContent = 'AP Mode';
            this.wifiIcon.textContent = '⚠️';
            this.statusElement.textContent = `In AP Mode: ${status.apName} (IP: ${status.apIP})`;
            this.statusElement.className = 'ap-mode';
        } else {
            this.wifiStatus.textContent = 'Not connected';
            this.wifiIcon.textContent = '❌';
            this.statusElement.textContent = 'Not connected to WiFi';
            this.statusElement.className = 'disconnected';
        }

        // Update server status
        if (status.serverDiscovered) {
            this.serverStatus.textContent = `Found at ${status.serverIP}:${status.serverPort}`;
            this.serverIcon.textContent = '✅';
        } else {
            this.serverStatus.textContent = 'Not found';
            this.serverIcon.textContent = '❌';
        }

        // Update connection panel
        document.getElementById('connectionStatus').textContent = status.isAPMode ? 'AP Mode' : 'Connected';
        document.getElementById('sensorNameStatus').textContent = status.sensorName || 'Not set';
        document.getElementById('wifiStatus').textContent = status.isAPMode 
            ? `AP Mode (${status.apName || 'N/A'})`
            : (status.ssid || 'Not connected');
        document.getElementById('serverStatus').textContent = status.serverConnected ? 'Connected' : 'Not found';

        // Add IP address display
        document.getElementById('ipAddress').textContent = status.ip || status.apIP || 'N/A';

        // Add last configured SSID display
        document.getElementById('lastConfiguredSSID').textContent = status.lastConfiguredSSID || 'N/A';
        document.getElementById('mdnsHostname').textContent = status.mdnsHostname || 'N/A';

        // Update status icons
        document.getElementById('sensorNameIcon').textContent = status.sensorName ? '🟢' : '⚪';
        document.getElementById('wifiIcon').textContent = status.isAPMode || status.ssid ? '🟢' : '⚪';
        document.getElementById('serverIcon').textContent = status.serverConnected ? '🟢' : '⚪';

        console.log('Status updated');
    }

    updateStepVisibility(status) {
        if (status.sensorName) {
            this.sensorNameInput.value = status.sensorName;
            this.sensorNameStatus.textContent = status.sensorName;
            this.sensorNameIcon.textContent = '✅';
        }

        if (status.connected) {
            this.wifiStatus.textContent = status.ssid;
            this.wifiIcon.textContent = '✅';
        }

        if (status.serverDiscovered) {
            this.serverStatus.textContent = `Found at ${status.serverIP}:${status.serverPort}`;
            this.serverIcon.textContent = '✅';
        }
    }

    updateSensorNameFeedback() {
        const inputName = this.sensorNameInput.value;
        const sanitizedName = this.sanitizeName(inputName);
        const wifiName = `stairled-sensor-${sanitizedName}`;
        const mdnsName = `stairled-sensor-${sanitizedName}.local`;

        let feedbackHtml = `
            <p>Sanitized name: ${sanitizedName}</p>
            <p>WiFi AP name: ${wifiName}</p>
            <p>mDNS name: ${mdnsName}</p>
        `;

        if (wifiName.length > 32) {
            feedbackHtml += `<p style="color: red;">Warning: WiFi AP name is too long (max 32 characters)</p>`;
        }

        if (mdnsName.length > 63) {
            feedbackHtml += `<p style="color: red;">Warning: mDNS name is too long (max 63 characters)</p>`;
        }

        this.sensorNameFeedback.innerHTML = feedbackHtml;
    }

    sanitizeName(name) {
        return name.toLowerCase()
            .replace(/\s+/g, '-')
            .replace(/[^a-z0-9-]/g, '');
    }

    async setSensorName() {
        const inputName = this.sensorNameInput.value.trim();
        const sanitizedName = this.sanitizeName(inputName);
        
        if (!sanitizedName) {
            alert('Please enter a valid sensor name. only lowercase, numbers and dashes are allowed');
            this.sensorNameInput.focus();
            return;
        }
        try {
            const response = await this.configManager.saveConfig({ sensorName: sanitizedName });
            console.log('Sensor name save response:', response);
            if (response.needsReboot) {
                const newHotspotName = `stairled-sensor-${sanitizedName}`;
                alert(`Sensor name updated to "${sanitizedName}". The device will now restart and create a new WiFi hotspot named "${newHotspotName}". Please connect to the new hotspot to continue setup.`);
            }
            this.sensorNameStatus.textContent = sanitizedName;
            this.sensorNameIcon.textContent = '✅';
            this.showSetupStep(2);
            this.sensorData.stopPolling(); // Stop sensor polling when moving to WiFi setup
        } catch (error) {
            console.error('Failed to set sensor name:', error);
            alert('Failed to set sensor name. Please try again.');
        }
    }

    determineSetupStep(status) {
        if (!status.sensorName) {
            this.showSetupStep(1);
        } else if (!status.connected) {
            this.showSetupStep(2);
        } else if (status.serverDiscovered) {
            this.showSetupStep(3);
        } else {
            this.showSetupStep(2);
        }
        console.log('Current step:', this.currentStep);
    }

    showSetupStep(stepNumber) {
        this.currentStep = stepNumber;
        this.setupSteps.forEach((step, index) => {
            const shouldShow = index + 1 <= stepNumber;
            step.hidden = !shouldShow;
            console.log(`Step ${index + 1} visibility:`, shouldShow);
        });
    }

    displayError(message) {
        this.statusElement.textContent = message;
        this.statusElement.className = 'error';
        console.error('Error displayed:', message);
    }
}
