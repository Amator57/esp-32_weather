function getQueryParam(name) {
  const url = new URL(window.location.href);
  return url.searchParams.get(name);
}

document.addEventListener("DOMContentLoaded", () => {
  const info = document.getElementById("info");
  const date = getQueryParam("date");

  if (!date) {
    info.innerText = "❗ Не вказано параметр ?date=YYYY-MM-DD";
    return;
  }

  const path = `/log/${date}.csv`;
  fetch(path)
    .then(res => {
      if (!res.ok) throw new Error("Файл не знайдено");
      return res.text();
    })
    .then(text => {
      const lines = text.trim().split("\n");
      const temps = [], hums = [], press = [];

      for (let i = 1; i < lines.length; i++) {
        const [time, t, h, p] = lines[i].split(",");
        const ts = new Date(`${date}T${time}`);
        const temp = parseFloat(t);
        const hum = parseFloat(h);
        const pres = parseFloat(p);

        if (!isNaN(temp) && !isNaN(hum) && !isNaN(pres)) {
          temps.push({ x: ts, y: temp });
          hums.push({ x: ts, y: hum });
          press.push({ x: ts, y: pres });
        }
      }

      if (temps.length < 2) {
        throw new Error("Недостатньо коректних даних для побудови графіка");
      }

      const allX = temps.map(p => p.x);
      const minTime = new Date(Math.min(...allX));
      const maxTime = new Date(Math.max(...allX));

      const getMinMax = data => {
        const ys = data.map(p => p.y);
        const min = Math.min(...ys);
        const max = Math.max(...ys);
        const margin = (max - min) * 0.1 || 1;
        return [min - margin, max + margin];
      };

      const [tempMin, tempMax] = getMinMax(temps);
      const [humMin, humMax]   = getMinMax(hums);
      const [presMin, presMax] = getMinMax(press);

      info.innerText = `Графік за ${date}`;
      const ctx = document.getElementById('bmeChart').getContext('2d');

      new Chart(ctx, {
        type: 'line',
        data: {
          datasets: [
            {
              label: 'Температура (°C)',
              data: temps,
              borderColor: 'red',
              borderWidth: 1,
              yAxisID: 'y_temp'
            },
            {
              label: 'Вологість (%)',
              data: hums,
              borderColor: 'blue',
              borderWidth: 1,
              yAxisID: 'y_hum'
            },
            {
              label: 'Тиск (гПа)',
              data: press,
              borderColor: 'green',
              borderWidth: 1,
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
              type: 'time',
              min: minTime,
              max: maxTime,
              time: {
                unit: 'hour',
                displayFormats: { hour: 'HH:mm' }
              },
              title: { display: true, text: 'Час' }
            },
            y_temp: {
              type: 'linear',
              position: 'left',
              title: { display: true, text: 'Температура (°C)' },
              min: tempMin,
              max: tempMax,
              ticks: { color: 'red' }
            },
            y_hum: {
              type: 'linear',
              position: 'right',
              title: { display: true, text: 'Вологість (%)' },
              min: humMin,
              max: humMax,
              ticks: { color: 'blue' },
              grid: { drawOnChartArea: false }
            },
            y_pres: {
              type: 'linear',
              position: 'right',
              offset: true,
              title: { display: true, text: 'Тиск (гПа)' },
              min: presMin,
              max: presMax,
              ticks: { color: 'green' },
              grid: { drawOnChartArea: false }
            }
          }
        }
      });
    })
    .catch(err => {
      info.innerText = "❌ Помилка: " + err.message;
    });
});
