class ChartManager {
    constructor(sensorData) {
        this.sensorData = sensorData;
        this.chartElement = document.getElementById('sensorChart');
        this.options = {
            lines: [{ data: [], color: '#61dafb' }],  // Light blue color for the line
            xAxis: { 
                title: 'Time', 
                min: 0, 
                max: this.sensorData.maxDataPoints, 
                ticks: 10,
                color: '#e0e0e0'  // Light gray color for axis labels
            },
            yAxis: { 
                title: 'Sensor Value', 
                min: 0, 
                max: 1000, 
                ticks: 100,
                color: '#e0e0e0'  // Light gray color for axis labels
            },
            backgroundColor: '#2c2c2c',  // Dark background color
            gridColor: '#444444'  // Darker gray for grid lines
        };
    }

    initChart() {
        // Use the original Chart function with our custom options
        Chart('#sensorChart', this.options);
    }

    updateChart() {
        // Update the data in the options
        this.options.lines[0].data = this.sensorData.data.filter(point => point[1] !== null && !isNaN(point[1]));
        
        // Recalculate min and max for x-axis if needed
        if (this.options.lines[0].data.length > 0) {
            this.options.xAxis.min = Math.min(...this.options.lines[0].data.map(point => point[0]));
            this.options.xAxis.max = Math.max(...this.options.lines[0].data.map(point => point[0]));
        }

        // Redraw the chart
        this.initChart();
    }
}