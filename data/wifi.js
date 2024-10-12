let selectedNetwork = null;

async function scanWiFi() {
    try {
        let response = await fetch('/api/wifi-scan');
        let networks = await response.json();
        networks.sort((a,b) => b.rssi - a.rssi)
        const wifiNetworksDiv = document.getElementById('wifiNetworks');
        wifiNetworksDiv.innerHTML = '';
        networks.forEach(network => {
            const button = document.createElement('button');
            button.innerHTML = `<span class='ssid'>${network.ssid}</span> <span class='rssi'>(${network.rssi} dBm)</span>`;
            button.classList.add('wifi-network');
            button.onclick = () => selectNetwork(network.ssid, button);
            wifiNetworksDiv.appendChild(button);
        });
        
      } catch (error) {
          console.error('Error scanning WiFi:', error);
      }
}

function selectNetwork(ssid, button) {
    if (selectedNetwork) {
        selectedNetwork.classList.remove('selected');
    }
    selectedNetwork = button;
    button.classList.add('selected');

    const password = prompt('Enter password for ' + ssid);
    if (password !== null) {
        connectWiFi(ssid, password);
    }
}

function connectWiFi(ssid, password) {
    fetch('/api/wifi-connect', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/x-www-form-urlencoded',
        },
        body: `ssid=${encodeURIComponent(ssid)}&password=${encodeURIComponent(password)}`
    })
    .then(response => response.text())
    .then(result => {
        alert(result);
    })
    .catch(error => console.error('Error connecting to WiFi:', error));
}

// Initialize
document.addEventListener('DOMContentLoaded', function() {
    document.getElementById('scanWifiButton').addEventListener('click', scanWiFi);
});