### GRAFANA INFLUXDB SETUP

This is a simple setup for a Grafana dashboard with InfluxDB as the backend. The setup is based on the following docker images:

* [Grafana](https://hub.docker.com/r/grafana/grafana/)

* [InfluxDB](https://hub.docker.com/_/influxdb/)

## Setup

### 1. Clone the repository

```bash
git clone https://github.com/dhsosa98/grafana-influxDB-server
```

### 2. Run the docker-compose file

```bash
docker-compose up -d
```

### 3. Install python dependencies

```bash
pip install -r requeriments.txt
```

### 4. Run the python script

```bash
python3 main.py
```

### 5. Open the Grafana dashboard

Open the browser and go to `localhost:3000`. The default username and password are `admin` and `admin`.
