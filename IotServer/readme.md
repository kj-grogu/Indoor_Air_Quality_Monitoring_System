## How to use requirements.txt

pip install -r /path/to/requirements.txt

## Devices 

devices related comments/ mentions here


now device(humidifier) is controlled using ESP32

## UX

HTML + CSS + FORMS + TABLE + ALERts

used for user interface.

## Server 

connect to the instance in AWS using the below command 


`ssh -i "bprakash2-ubuntu.pem" ubuntu@ec2-34-217-96-154.us-west-2.compute.amazonaws.com`

navigate to the dir 'helloiot'

activate the virtual environment

`source venv/bin/activate`

make changes to the `app.py` file this is our web server


restart the server using below command

`ps -C gunicorn fch -o pid | head -n 1`

`kill -s HUP <pid_from_above>`


## MySQL DB

To access database 

once you are ssh'ed into the machine, run below command

`sudo mysql -u root -p`

enter password

list all the databases

`show databases;`

select the iot315 database using below command

`use iot315`

list all the tables in the iot315 database

`show tables`

look at the sensor data that is being sent in by the devices

`select * from SensorData;`

we have used app credentials for the iot315@gmail.com user account to send emails.
