function fetchBMEData() {
    fetch('/bme_data')
        .then(response => response.json())
        .then(data => {
            document.getElementById('temperature').innerText = data.temperature.toFixed(2);
            document.getElementById('humidity').innerText = data.humidity.toFixed(2);
            document.getElementById('pressure').innerText = data.pressure.toFixed(2);
            document.getElementById('presmmhg').innerText = data.presmmhg.toFixed(2);
        })
        .catch(error => {
            console.error('Помилка отримання даних BME280:', error);
        });
}

async function loadChart() {
    try {
        const res = await fetch('/bme_chart_data');
        const data = await res.json();

        const labels = data.map(item => item.time);
        const temps = data.map(item => item.temperature);
        const hums = data.map(item => item.humidity);
        const press = data.map(item => item.pressure);

        const minTemp = Math.min(...temps);
        const maxTemp = Math.max(...temps);
        const minHum = Math.min(...hums);
        const maxHum = Math.max(...hums);
        const minPres = Math.min(...press);
        const maxPres = Math.max(...press);

        const ctx = document.getElementById('bmeChart').getContext('2d');
        if (window.myChart) window.myChart.destroy();
        window.myChart = new Chart(ctx, {
            type: 'line',
            data: {
                labels: labels,
                datasets: [
                    {
                        label: 'Температура (°C)',
                        data: temps,
                        borderColor: 'red',
                        borderWidth: 1.5,
                        fill: false,
                        yAxisID: 'y_temp'
                    },
                    {
                        label: 'Вологість (%)',
                        data: hums,
                        borderColor: 'blue',
                        borderWidth: 1.5,
                        fill: false,
                        yAxisID: 'y_hum'
                    },
                    {
                        label: 'Тиск (гПа)',
                        data: press,
                        borderColor: 'green',
                        borderWidth: 1.5,
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
                plugins: { // Додано плагін масштабування та прокрутки
                    zoom: {
                        pan: {
                            enabled: true,
                            mode: 'x', // Прокрутка лише по осі X
                            threshold: 5 // Мінімальний рух для прокрутки
                        },
                        zoom: {
                            wheel: {
                                enabled: true, // Масштабування колесом миші
                            },
                            pinch: {
                                enabled: true // Масштабування щипком (на сенсорних екранах)
                            },
                            mode: 'x', // Масштабування лише по осі X
                        }
                    }
                },
                scales: {
                    x: {
                        type: 'time',
                        time: {
                            unit: 'minute',
                            tooltipFormat: 'HH:mm:ss',
                            displayFormats: {
                                minute: 'HH:mm'
                            }
                        },
                        // Встановлюємо початковий вигляд на останню годину
                        min: data.length > 0 ? new Date(new Date(labels[labels.length - 1]).getTime() - 60 * 60 * 1000).toISOString() : undefined,
                        max: data.length > 0 ? new Date(labels[labels.length - 1]).toISOString() : undefined,
                        title: {
                            display: true,
                            text: 'Час'
                        }
                    },
                    y_temp: {
                        type: 'linear',
                        position: 'left',
                        title: { display: true, text: 'Температура (°C)' },
                        ticks: { color: 'red' },
                        suggestedMin: Math.floor(minTemp - 1),
                        suggestedMax: Math.ceil(maxTemp + 1)
                    },
                    y_hum: {
                        type: 'linear',
                        position: 'right',
                        title: { display: true, text: 'Вологість (%)' },
                        ticks: { color: 'blue' },
                        suggestedMin: Math.floor(minHum - 1),
                        suggestedMax: Math.ceil(maxHum + 1),
                        grid: { drawOnChartArea: false }
                    },
                    y_pres: {
                        type: 'linear',
                        position: 'right',
                        offset: true,
                        title: { display: true, text: 'Тиск (гПа)' },
                        ticks: { color: 'green' },
                        suggestedMin: Math.floor(minPres - 1),
                        suggestedMax: Math.ceil(maxPres + 1),
                        grid: { drawOnChartArea: false }
                    }
                }
            }
        });
    } catch (e) {
        console.error("Помилка завантаження графіка:", e);
    }
}

fetchBMEData();
loadChart();
setInterval(fetchBMEData, 60000);
setInterval(loadChart, 60000);