import { initializeApp } from "https://www.gstatic.com/firebasejs/10.9.0/firebase-app.js";
import {
  getDatabase,
  get,
  ref,
  child,
} from "https://www.gstatic.com/firebasejs/10.9.0/firebase-database.js";

const firebaseConfig = {
  apiKey: "AIzaSyDy_UzoZ85N0zmNGRGvZh1OijDmd13V0QA",
  authDomain: "waste-water-stats.firebaseapp.com",
  databaseURL:
    "https://waste-water-stats-default-rtdb.asia-southeast1.firebasedatabase.app",
  projectId: "waste-water-stats",
  storageBucket: "waste-water-stats.appspot.com",
  messagingSenderId: "196733197128",
  appId: "1:196733197128:web:224e4290f9da202acaf17d",
};


const app = initializeApp(firebaseConfig);

const db = getDatabase(app);

const dbRef = ref(db);

function getNewSeries() {
  return get(child(dbRef, "/data"))
    .then((snapshot) => {
      const data = {
        turbidity: [],
        tds: [],
        phVal: [],
        temp: [],
        condition: [],
        dates: [],
      };
      snapshot.forEach((childSnapshot) => {
        data.turbidity.push(childSnapshot.val().turbidity);
        data.tds.push(childSnapshot.val().tdsvalue);
        data.phVal.push(childSnapshot.val().PHvalue);
        data.temp.push(childSnapshot.val().temperature);
        data.condition.push(childSnapshot.val().Condition ? 1 : 0);
        data.dates.push(childSnapshot.val().time);
      });
      console.log(data.turbidity);
      return data;
    })
    .catch((error) => {
      console.error("Error retrieving data from Firebase: ", error);
    });
}

function generateChart(data) {
  data.forEach(
    ({
      elementId,
      title,
      values,
      dates,
      color,
      unit,
      threshold1,
      threshold2,
      anno,
    }) => {
      const chartOptions = createChartOptions(
        title,
        values,
        dates,
        color,
        unit,
        threshold1,
        threshold2,
        anno
      );
      var chart = new ApexCharts(
        document.querySelector("#" + elementId),
        chartOptions
      );
      chart.render();
    }
  );
}

getNewSeries()
  .then((data) => {
    generateChart([
      {
        elementId: "ph-area-chart",
        title: "PH",
        values: data.phVal,
        dates: data.dates,
        color: "#2e7d32",
        unit: "PH",
        threshold1: 6.0,
        threshold2: 8.5,
        anno: "Standard PH range",
        
      },
      {
        elementId: "temperature-area-chart",
        title: "Temperature",
        values: data.temp,
        dates: data.dates,
        color: "#d50000",
        unit: "Celsius",
        threshold1: 40,
        threshold2: null,
        anno: "Standard temperature level - 40C",
      },
      {
        elementId: "turbudity-area-chart",
        title: "Turbudity",
        values: data.turbidity,
        dates: data.dates,
        color: "#2962ff",
        unit: "NTU",
        threshold1: 150,
        threshold2: null,
        anno: "Standard turbudity level 150 NTU",
      },
      {
        elementId: "tds-area-chart",
        title: "Total Dissolved Solids",
        values: data.tds,
        dates: data.dates,
        color: "#ff6d00",
        unit: "mg/L",
        threshold1: 1500,
        threshold2: null,
        anno: "Standard TDS level - 1500mg/L",
      },
      {
        elementId: "condition-area-chart",
        title: "Condition",
        values: data.condition,
        dates: data.dates,
        color: "#ff6d00",
        unit: "Status",
        threshold1: 1,
        anno: "Condition",
      },
    ]);
  })
  .catch((error) => {
    console.error("Error retrieving data from Firebase: ", error);
  });

function createChartOptions(
  name,
  data,
  dates,
  color,
  unit,
  threshold1,
  threshold2,
  anno
) {
  return {
    series: [{ name, data }],
    chart: {
      type: "area",
      background: "transparent",
      height: 350,
      stacked: false,
      toolbar: {
        show: false,
      },
      animations: {
        enabled: true,
        easing: "linear",
        dynamicAnimation: {
          speed: 1000,
        },
      },
      dropShadow: {
        enabled: true,
        opacity: 0.3,
        blur: 5,
        left: -7,
        top: 22,
      },
    },
    colors: [color],
    labels: dates,
    dataLabels: {
      enabled: false,
    },
    fill: {
      gradient: {
        opacityFrom: 0.4,
        opacityTo: 0.1,
        shadeIntensity: 1,
        stops: [0, 100],
        type: "vertical",
      },
      type: "gradient",
    },
    grid: {
      borderColor: "#55596e",
      yaxis: {
        lines: {
          show: true,
        },
      },
      xaxis: {
        lines: {
          show: true,
        },
      },
    },
    markers: {
      size: 6,
      strokeColors: "#1b2635",
      strokeWidth: 3,
    },
    stroke: {
      curve: "smooth",
    },
    xaxis: {
      axisBorder: {
        color: "#55596e",
        show: true,
      },
      axisTicks: {
        color: "#55596e",
        show: true,
      },
      labels: {
        offsetY: 5,
        style: {
          colors: "#f5f7ff",
        },
      },
    },
    yaxis:
      name === "Condition"
        ? {
            tickAmount: 1,
            title: {
              text: unit,
              style: {
                color: "#f5f7ff",
              },
            },
            labels: {
              formatter: function (val) {
                return val === 1 ? "Good" : "Bad";
              },
              style: {
                colors: ["#f5f7ff"],
              },
            },
          }
        : {
            title: {
              text: unit,
              style: {
                color: "#f5f7ff",
              },
            },
            labels: {
              style: {
                colors: ["#f5f7ff"],
              },
            },
          },

    tooltip: {
      shared: true,
      intersect: false,
      theme: "dark",
    },
    annotations: {
      yaxis: [
        {
          y: threshold1,
          y2: threshold2,
          borderColor: "#FFFF00",
          label: {
            borderColor: "#FFFF00",
            style: {
              color: "#000",
              background: "#FFFF00",
            },
            text: anno,
          },
        },
      ],
    },
  };
}



