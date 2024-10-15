class SensorData {
    constructor(maxDataPoints = 100) {
        this.data = [];
        this.maxDataPoints = maxDataPoints;
        this.lastFetchTime = 0;
        this.fetchInterval = 150; // 150ms
        this.isPending = false;
        this.pollingInterval = null; // Add this line
    }

    async fetch() {
        const now = performance.now();
        if (now - this.lastFetchTime < this.fetchInterval || this.isPending) {
            return;
        }
        this.lastFetchTime = now;
        this.isPending = true;

        try {
            const response = await fetch('/api/sensor-data');
            const value = await response.text();
            this.addDataPoint(parseInt(value));
        } catch (error) {
            console.error('Error fetching sensor data:', error);
        } finally {
            this.isPending = false;
        }
    }

    addDataPoint(value) {
        if (value !== null && !isNaN(value)) {
            this.data.push([this.data.length, value]);
            if (this.data.length > this.maxDataPoints) {
                this.data.shift();
                // Recalculate x-values
                this.data = this.data.map((point, index) => [index, point[1]]);
            }
        }
    }

    startPolling(interval = 1000) {
        this.stopPolling(); // Ensure any existing interval is cleared
        this.pollingInterval = setInterval(() => this.fetch(), interval);
    }

    stopPolling() {
        if (this.pollingInterval) {
            clearInterval(this.pollingInterval);
            this.pollingInterval = null;
        }
    }
}
