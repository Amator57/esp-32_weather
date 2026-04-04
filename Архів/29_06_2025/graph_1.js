function fetchLogData() {
    fetch('/log_chart_data')
        .then(response => response.json())
        .then(data => drawLogChart(data))
        .catch(error => console.error('Помилка отримання архіву:', error));
}

function getMinMax(data) {
    const min = Math.min(...data);
    const max = Math.max(...data);
    const padding = (max - min) * 0.1 || 1;
    return { min: min - padding, max: max + padding };
}

function drawLogChart(data) {
    const labels = data.map(item => item.time);
    const temps = data.map(item => item.temperature);
    const hums = data.map(item => item.humidity);
    const press = data.map(item => item.pressure);

    const tempBounds = getMinMax(temps);
    const humBounds = getMinMax(hums);
    const pressBounds = getMinMax(press);

    const ctx = document.getElementById('logChart').getContext('2d');
    if (window.logChart) window.logChart.destroy();
    window.logChart = new Chart(ctx, {
        type: 'line',
        data: {
            labels: labels,
            datasets: [
                {
                    label: 'Температура (°C)',
                    data: temps,
                    borderColor: 'red',
                    fill: false,
                    yAxisID: 'y_temp'
                },
                {
                    label: 'Вологість (%)',
                    data: hums,
                    borderColor: 'blue',
                    fill: false,
                    yAxisID: 'y_hum'
                },
                {
                    label: 'Тиск (гПа)',
                    data: press,
                    borderColor: 'green',
                    fill: false,
                    yAxisID: 'y_pres'
                }
            ]
        },
        options: {
            responsive: true,
            interaction: {
                mode: 'index',
                intersect: false
            },
            stacked: false,
            scales: {
                x: {
                    display: true,
                    title: { display: true, text: 'Час' }
                },
                y_temp: {
                    type: 'linear',
                    position: 'left',
                    title: { display: true, text: 'Температура (°C)' },
                    ticks: { color: 'red' },
                    min: tempBounds.min,
                    max: tempBounds.max
                },
                y_hum: {
                    type: 'linear',
                    position: 'right',
                    title: { display: true, text: 'Вологість (%)' },
                    ticks: { color: 'blue' },
                    min: humBounds.min,
                    max: humBounds.max,
                    grid: { drawOnChartArea: false }
                },
                y_pres: {
                    type: 'linear',
                    position: 'right',
                    offset: true,
                    title: { display: true, text: 'Тиск (гПа)' },
                    ticks: { color: 'green' },
                    min: pressBounds.min,
                    max: pressBounds.max,
                    grid: { drawOnChartArea: false }
                }
            }
        }
    });
}

fetchLogData();
