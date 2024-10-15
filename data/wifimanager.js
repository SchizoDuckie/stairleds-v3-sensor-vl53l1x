class WifiManager {
    constructor() {
        this.wifiNetworksDiv = document.getElementById('wifiNetworks');
        this.scanWifiButton = document.getElementById('scanWifi');
        this.wifiModal = document.getElementById('wifiModal');
        this.closeWifiModalButton = document.getElementById('closeWifiModal');
        this.connectionModal = document.getElementById('connectionModal');
        this.connectionStatus = document.getElementById('connectionStatus');
        this.refreshPageButton = document.getElementById('refreshPage');
        this.selectedNetwork = null;

        this.bindEvents();
    }

    bindEvents() {
        this.scanWifiButton.addEventListener('click', () => this.scanWiFi());
        this.closeWifiModalButton.addEventListener('click', () => this.closeWifiModal());
        this.refreshPageButton.addEventListener('click', () => window.location.reload());
    }


    async scanWiFi() {
        try {
            this.scanWifiButton.disabled = true;
            this.scanWifiButton.textContent = 'Scanning...';
            const response = await fetch('/api/wifi-scan');
            const networks = await response.json();
            this.displayNetworks(networks);
            this.openWifiModal();
        } catch (error) {
            console.error('Error scanning WiFi:', error);
            alert('Failed to scan WiFi networks. Please try again.');
        } finally {
            this.scanWifiButton.disabled = false;
            this.scanWifiButton.textContent = 'Scan for Networks';
        }
    }

     displayNetworks(networks) {
        this.wifiNetworksDiv.innerHTML = '';
        if (networks.length === 0) {
            this.wifiNetworksDiv.innerHTML = '<p>No networks found. Please try scanning again.</p>';
            return;
        }

        networks.sort((a, b) => b.rssi - a.rssi);
        
        networks.forEach(network => {
            const button = document.createElement('button');
            button.classList.add('wifi-network');
            if (network.ssid.toLowerCase().includes('stairled-server')) {
                button.classList.add('stairled-server');
            }
            button.innerHTML = `
                <span class='ssid'>${network.ssid}</span>
                <div class='wifi-signal-container'>
                    <div class='wifi-signal ${this.getSignalStrength(network.rssi)}'>
                        <div class='bar'></div>
                        <div class='bar'></div>
                        <div class='bar'></div>
                        <div class='bar'></div>
                    </div>
                    <span class='wifi-dbm'>${network.rssi} dBm</span>
                </div>
            `;
            button.addEventListener('click', () => this.selectNetwork(network.ssid, button));
            this.wifiNetworksDiv.appendChild(button);
        });
    }

    getSignalStrength(rssi) {
        if (rssi >= -50) return 'excellent';
        if (rssi >= -60) return 'good';
        if (rssi >= -70) return 'fair';
        return 'weak';
    }
    
    selectNetwork(ssid, button) {
        if (this.selectedNetwork) {
            this.selectedNetwork.classList.remove('selected');
        }
        this.selectedNetwork = button;
        button.classList.add('selected');

        const password = prompt(`Enter password for ${ssid}`);
        if (password !== null) {
            this.closeWifiModal();
            this.connectWiFi(ssid, password);
        }
    }

       async connectWiFi(ssid, password) {
        this.openConnectionModal();
        this.connectionStatus.textContent = 'Attempting to connect...';
        
        try {
            const response = await fetch('/api/wifi-connect', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/x-www-form-urlencoded',
                },
                body: `ssid=${encodeURIComponent(ssid)}&password=${encodeURIComponent(password)}`
            });

            if (response.ok) {
                this.handleSuccessfulConnection(ssid);
            } else {
                const errorText = await response.text();
                this.connectionStatus.textContent = `Failed to connect: ${errorText}`;
                this.refreshPageButton.hidden = false;
            }
        } catch (error) {
            console.error('Error connecting to WiFi:', error);
            this.connectionStatus.textContent = 'Failed to connect. Please try again.';
            this.refreshPageButton.hidden = false;
        }
    }

    handleSuccessfulConnection(ssid) {
        this.connectionStatus.innerHTML = `
            <p>Successfully connected to ${ssid}. The device will now reboot to apply the new settings.</p>
            <p>Please follow these steps:</p>
            <ol>
                <li>Wait about 30 seconds for the device to reboot.</li>
                <li>Connect your device (phone/computer) to the "${ssid}" network.</li>
                <li>Once connected, click the "Refresh Page" button below.</li>
            </ol>
            <p>If you don't see the "${ssid}" network, wait a bit longer and check your available WiFi networks again.</p>
        `;
        this.refreshPageButton.hidden = false;
    }

    openConnectionModal() {
        this.connectionModal.style.display = 'block';
    }

    closeConnectionModal() {
        this.connectionModal.style.display = 'none';
    }

    openWifiModal() {
        this.wifiModal.style.display = 'block';
    }

    closeWifiModal() {
        this.wifiModal.style.display = 'none';
    }
}