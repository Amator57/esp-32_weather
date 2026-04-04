document.addEventListener("DOMContentLoaded", function () {
  const chartCanvas = document.getElementById("chart");
  const errorDiv = document.getElementById("error");

  const urlParams = new URLSearchParams(window.location.search);
  const date = urlParams.get("date");

  if (!date) {
    errorDiv.innerHTML = "❌ Помилка: Не вказано дату в параметрах URL (?date=YYYY-MM-DD)";
    return;
  }

  fetch(`/log/${date}.csv`)
    .then(response => {
      if (!response.ok) throw new Error("Файл не знайдено");
      return response.text();
    })
    .then(csv => {
      const lines = csv.trim().split("\n");
      const labels = [];
      const temperatureData = [];
      const humidityData = [];
      const pressureData = [];

      for (let line of lines) {
        const [time, tempStr, humStr, pressStr] = line.split(",");
        const temp = parseFloat(tempStr);
        const hum = parseFloat(humStr);
        const press = parseFloat(pressStr);

        if (
          time &&
          !isNaN(temp) && temp !== 0 &&
          !isNaN(hum) && hum !== 0 &&
          !isNaN(press) && press !== 0
        ) {
          labels.push(time);
          temperatureData.push(temp);
          humidityData.push(hum);
          pressureData.push(press);
        }
      }

      if (labels.length < 2) {
        errorDiv.innerHTML = "❌ Помилка: Недостатньо коректних даних для побудови графіка (мінімум 2 точки).";
        return;
      }

      // Знайти min/max для кожної осі
      const minTemp = Math.min(...temperatureData);
      const maxTemp = Math.max(...temperatureData);
      const minHum = Math.min(...humidityData);
      const maxHum = Math.max(...humidityData);
      const minPress = Math.min(...pressureData);
      const maxPress = Math.max(...pressureData);

      const ctx = chartCanvas.getContext("2d");
      new Chart(ctx, {
        type: "line",
        data: {
          labels,
          datasets: [
            {
              label: "Температура (°C)",
              data: temperatureData,
              borderColor: "red",
              borderWidth: 1.5,
              fill: false,
              yAxisID: "yTemp"
            },
            {
              label: "Вологість (%)",
              data: humidityData,
              borderColor: "blue",
              borderWidth: 1.5,
              fill: false,
              yAxisID: "yHum"
            },
            {
              label: "Тиск (hPa)",
              data: pressureData,
              borderColor: "green",
              borderWidth: 1.5,
              fill: false,
              yAxisID: "yPress"
            }
          ]
        },
        options: {
          responsive: true,
          maintainAspectRatio: false,
          scales: {
            x: {
              title: {
                display: true,
                text: "Час"
              }
            },
            yTemp: {
              type: "linear",
              position: "left",
              min: Math.floor(minTemp - 1),
              max: Math.ceil(maxTemp + 1),
              title: {
                display: true,
                text: "Температура (°C)"
              }
            },
            yHum: {
              type: "linear",
              position: "right",
              min: Math.floor(minHum - 1),
              max: Math.ceil(maxHum + 1),
              title: {
                display: true,
                text: "Вологість (%)"
              },
              grid: {
                drawOnChartArea: false
              }
            },
            yPress: {
              type: "linear",
              position: "right",
              min: Math.floor(minPress - 1),
              max: Math.ceil(maxPress + 1),
              title: {
                display: true,
                text: "Тиск (hPa)"
              },
              grid: {
                drawOnChartArea: false
              }
            }
          },
          interaction: {
            mode: "index",
            intersect: false
          },
          plugins: {
            legend: {
              position: "top"
            },
            title: {
              display: true,
              text: "Графік BME280 з архіву"
            }
          }
        }
      });
    })
    .catch(err => {
      errorDiv.innerHTML = `❌ Помилка: ${err.message}`;
    });
});
