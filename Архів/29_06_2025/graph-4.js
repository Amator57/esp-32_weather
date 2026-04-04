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
      if (!response.ok) {
        throw new Error("Файл не знайдено");
      }
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
              title: {
                display: true,
                text: "Температура (°C)"
              },
              beginAtZero: false
            },
            yHum: {
              type: "linear",
              position: "right",
              title: {
                display: true,
                text: "Вологість (%)"
              },
              beginAtZero: false,
              grid: {
                drawOnChartArea: false
              }
            },
            yPress: {
              type: "linear",
              position: "right",
              title: {
                display: true,
                text: "Тиск (hPa)"
              },
              beginAtZero: false,
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
      errorDiv.innerHTML = "❌ Помилка: " + err.message;
    });
});
