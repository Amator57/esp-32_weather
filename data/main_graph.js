document.addEventListener("DOMContentLoaded", function () {
  const ctx = document.getElementById("chart").getContext("2d");

  fetch("/bme_chart_data")
    .then(response => response.json())
    .then(data => {
      if (data.length < 2) {
        document.getElementById("error").textContent = "Недостатньо даних для побудови графіка.";
        return;
      }

      const labels = data.map(d => d.time);
      const temperatureData = data.map(d => d.temperature);
      const humidityData = data.map(d => d.humidity);
      const pressureData = data.map(d => d.pressure);

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
              title: { display: true, text: "Час" }
            },
            yTemp: {
              type: "linear",
              position: "left",
              title: { display: true, text: "Температура (°C)" },
              beginAtZero: false
            },
            yHum: {
              type: "linear",
              position: "right",
              title: { display: true, text: "Вологість (%)" },
              beginAtZero: false,
              grid: { drawOnChartArea: false }
            },
            yPress: {
              type: "linear",
              position: "right",
              title: { display: true, text: "Тиск (hPa)" },
              beginAtZero: false,
              grid: { drawOnChartArea: false }
            }
          }
        }
      });
    })
    .catch(err => {
      document.getElementById("error").textContent = "Помилка завантаження графіка: " + err.message;
    });
});
