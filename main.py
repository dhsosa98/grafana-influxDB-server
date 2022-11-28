from fastapi import FastAPI
import uvicorn
from influxdb import InfluxDBClient
import datetime
from pydantic import BaseModel
import random

app = FastAPI()

@app.get("/")
async def root():
    return {"message": "Hello World"}

json_controllers_init = {
    "group1": {
        "actuators": [
            'led1',
            'led2',
            'led3',
            'stepper'
        ],
        "sensors": [
            'temp1',
            'hum1',
        ]
    },
    "group2": {
        "actuators": [
            'rele',
            'led1',
            'led2',
            'led3',
            'led4',
            'led5'
        ],
        "sensors": [
            'mic',
        ],
    },
    # todo: Falta agregar el resto de los grupos
    "group3": {
        "actuators": [],
        "sensors": [],
    },
    "group4": {
        "actuators": ['rele'],
        "sensors": ['proximity'],
    },
    "group5": {
        "actuators": ['stepper'],
        "sensors": ['fotorresistor'],
    },
    "group6": {
        "actuators": ['display', 'ledRGB', 'speaker'],
        "sensors": ['proximity'],
    },
    "group7": {
        "actuators": ['stepper', 'led1'],
        "sensors": ['fotosensor', 'joystick'],
    },

}

class Insert_Data_Body(BaseModel):
    group1: dict
    group2: dict
    group3: dict
    group4: dict
    group5: dict
    group6: dict
    group7: dict


# Endpoint para crear las bases de datos
@app.post("/create_databases")
async def create_databases():
    client = InfluxDBClient(host='localhost', port=8086, username='admin', password='admin123', database='test')
    for key in json_controllers_init.keys():
        for sensor in json_controllers_init[key]['sensors']:
            client.create_database(key + '_' + sensor)
        for actuator in json_controllers_init[key]['actuators']:
            client.create_database(key + '_' + actuator)
    return {"message": "Databases created"}

@app.post("/drop_databases")
async def drop_databases():
    client = InfluxDBClient(host='localhost', port=8086, username='admin', password='admin123', database='test')
    for key in json_controllers_init.keys():
        for sensor in json_controllers_init[key]['sensors']:
            client.drop_database(key + '_' + sensor)
        for actuator in json_controllers_init[key]['actuators']:
            client.drop_database(key + '_' + actuator)
    return {"message": "Databases dropped"}

# Endpoint para insertar los datos
@app.post("/insert_data")
async def insert_data(body: Insert_Data_Body):
    try:
        client = InfluxDBClient(host='localhost', port=8086, username='admin', password='admin123', database='test')
        for key in body.dict().keys():
            if key!='group3':
                
                for sensor in body.dict()[key]['sensors']:
                    json_body = [
                        {
                            "measurement": key + '_' + sensor['type'],
                            "time": datetime.datetime.utcnow(),
                            "fields": {
                                "value": sensor['current_value']
                            }
                        }
                    ]
                    client.write_points(json_body)
                for actuator in body.dict()[key]['actuators']:
                    json_body = [
                        {
                            "measurement": key + '_' + actuator['type'],
                            "time": datetime.datetime.utcnow(),
                            "fields": {
                                "value": actuator['current_value']
                            }
                        }
                    ]
                    client.write_points(json_body)
        return {"message": "Data inserted"}
    except Exception as e:
        print(e)
        return {"message": e}


# Endpoints para pruebas
@app.post("/create_test")
async def create_test():
    client = InfluxDBClient(host='localhost', port=8086, username='admin', password='admin123', database='test')
    client.create_database('test')
    return {"message": "Database created"}

@app.post("/drop_test")
async def drop_test():
    client = InfluxDBClient(host='localhost', port=8086, username='admin', password='admin123', database='test')
    client.drop_database('test')
    return {"message": "Database dropped"}


@app.post("/influx_test")
async def influx():
    try:
        client = InfluxDBClient('localhost', 8086, 'admin', 'admin123', 'test')
        iso = datetime.datetime.utcnow()
        json_body = [
        {
        "measurement": "test",
        "time": iso,
        "fields": {
        "value": random.randint(1, 100)
        }
        }
        ]
        client.write_points(json_body)
        json_body = [
        {
        "measurement": "test",
        "time": iso,
        "fields": {
        "medianat": random.randint(1, 100),
        }
        }
        ]
        client.write_points(json_body)

        return {"message": "Success"}
    except Exception as e:
        print(e)
        return {"message": e}

if (__name__ == "__main__"):
    uvicorn.run("main:app", host="0.0.0.0", port=8000, reload=True)
