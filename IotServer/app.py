from flask import *
from flask_mail import Mail, Message
import time
import datetime
import mysql.connector

app = Flask(__name__)
app.secret_key = 'zz-secret-key-zz'
mail_settings = {
    "MAIL_SERVER": 'smtp.gmail.com',
    "MAIL_PORT": 465,
    "MAIL_USE_TLS": False,
    "MAIL_USE_SSL": True,
    "MAIL_USERNAME": 'iot315coen@gmail.com',
    "MAIL_PASSWORD": 'wenuiqpcjxkrjqjc'
}
app.config.update(mail_settings)
mail = Mail(app)

sensordatafile = 'sensordata.log'
usersfile = "users.log"
humidifierStatus = "OFF"

@app.route('/')
def index():
    return render_template('login.html')

@app.route('/register', methods=['GET', 'POST'])
def register():
    if request.method == 'POST':
        # get user registration data from the form
        email = request.form['email']
        password = request.form['password']
        confirm_password = request.form['confirm_password']

        # check if the password and confirm password match
        if password != confirm_password:
            error_message = 'Passwords do not match'
            return render_template('registration.html', error_message=error_message)


        write_user(email, password)
        # redirect to the login page
        return redirect(url_for('login'))

    # display the registration form for GET requests
    return render_template('register.html')

@app.route('/login', methods=['GET', 'POST'])
def login():
    if request.method == 'POST':
        # check user credentials
        useremail = request.form['useremail']
        password = request.form['password']
        if check_user_credentials(useremail, password):
            session['useremail'] = useremail
            session['logged_in'] = True
            return redirect(url_for('dashboard'))
        else:
            # display error message
            error_message = 'Invalid useremail or password'
            return render_template('login.html', error_message=error_message)
    else:
        # display login form
        return render_template('login.html')

@app.route('/devicestatus', methods=['GET'])
def devicestatus():
    print("turning device on")
    return Response(humidifierStatus, status=200)

@app.route('/deviceon', methods=['POST'])
def deviceON():
    print("turning device on")
    global humidifierStatus
    humidifierStatus = "ON"
    return Response(humidifierStatus, status=200)

@app.route('/deviceoff', methods=['POST'])
def deviceOFF():
    print("turning device off")
    global humidifierStatus
    humidifierStatus = "OFF"
    return Response(humidifierStatus, status=200)

@app.route('/dashboard')
def dashboard():
    # check if the user is logged in
    if 'logged_in' in session and session['logged_in']:
        dashboard_data = {}
        dashboard_data['data'] = read_sensordata()
        dashboard_data['summary'] = read_summary()
        return render_template('dashboard.html', dashboard_data=dashboard_data)
    else:
        # redirect to login page if the user is not logged in
        return redirect(url_for('login'))

@app.route('/dashboard/data')
def dashboard_data():
    return read_sensordata()

@app.route('/report', methods=['POST'])
def sendreport():
    dashboard_data = {}
    dashboard_data['data'] = read_sensordata()
    dashboard_data['summary'] = read_summary()
    with app.app_context():
        table_html = render_template('table.html', dashboard_data=dashboard_data)
        msg = Message(subject="IoT Sensor Data report",
                      sender=app.config.get("MAIL_USERNAME"),
                      recipients=[session['useremail'], 'bprakash2@scu.edu'])
        msg.html = table_html
        res = mail.send(msg)
    return Response("OK", status=200)

def sendsmoke():
    with app.app_context():
        msg = Message(subject="COEN 315 IoT Sensor Data report",
                      sender=app.config.get("MAIL_USERNAME"),
                      recipients=['bprakash2@scu.edu'],  # replace with your email for testing
                      body="High Amount of smoke detected! Smoke metric above 500. Please take neccessary precautions!!")
        res = mail.send(msg)


def sendhumidity(humiditymsg):
    with app.app_context():
        msg = Message(subject="COEN 315 IoT Sensor Data report",
                      sender=app.config.get("MAIL_USERNAME"),
                      recipients=['bprakash2@scu.edu'],  # replace with your email for testing
                      body=f"{humiditymsg}")
        res = mail.send(msg)

@app.route('/sensordata', methods = ['GET', 'POST'])
def api_echo():
    if request.method == 'GET':
        res = json.dumps(read_sensordata())
        return Response(res, status=200)

    elif request.method == 'POST':
        payload = request.data.decode('utf-8')
        write_sensordata(payload)
        return Response("OK", status=200)

    else:
        data = {'message': 'Method not supported'}
        js = json.dumps(data)
        resp = Response(js, status=200, mimetype='application/json')
        return resp

def read_sensordata():
    db = mysql.connector.connect(
        host="localhost",
        user="iotuser",
        password="iotpwd",
        database="iot315"
    )

    cursor = db.cursor()
    query = "select * from SensorData ORDER BY Timestamp DESC LIMIT 200;"
    cursor.execute(query)
    data = cursor.fetchall()
    cursor.close()

    result = []
    for row in data:
        sensordata = {}
        sensordata['Metric'] = row[0]
        sensordata['Value'] = row[1]
        sensordata['Timestamp'] = str(row[2])
        result.append(sensordata)

    return result

def read_summary():
    db = mysql.connector.connect(
        host="localhost",
        user="iotuser",
        password="iotpwd",
        database="iot315"
    )

    cursor = db.cursor()
    query = "SELECT Metric, MIN(CAST(Value AS DOUBLE)) AS min, MAX(CAST(Value AS DOUBLE)) AS max, AVG(CAST(Value AS DOUBLE)) AS average FROM SensorData WHERE Timestamp >= DATE_SUB(NOW(), INTERVAL 24 HOUR) GROUP BY Metric;"
    #query = "select Metric, MIN(Value), MAX(Value), AVG(Value) from SensorData WHERE Timestamp >= DATE_SUB(NOW(), INTERVAL 24 HOUR) GROUP BY Metric;"
    cursor.execute(query)
    data = cursor.fetchall()
    cursor.close()

    result = []
    for row in data:
        sensordata = {}
        sensordata['Metric'] = row[0]
        sensordata['Min'] = row[1]
        sensordata['Max'] = row[2]
        sensordata['Avg'] = row[3]
        result.append(sensordata)

    return result

def write_sensordata(payload):
    db = mysql.connector.connect(
        host="localhost",
        user="iotuser",
        password="iotpwd",
        database="iot315"
    )
    cursor = db.cursor()

    data = payload[len("'SensorData="):-1]
    metrics = data.split(";")
    global humidifierStatus
    for metric in metrics:
        values = metric.split("-")
        print(values)
        query = "INSERT INTO SensorData (Metric, Value, TimeStamp) VALUES (%s, %s, %s)"
        values = (values[0], values[1], gettimestamp())
        cursor.execute(query, values)
        try:
            if values[0] == "Humidity":
                if float(values[1]) < 30.0:
                    humidifierStatus = "ON"
                    sendhumidity("Humidity below 30%, switching ON Humidifier.")
                elif float(values[1]) > 60.0:
                    humidifierStatus = "OFF"
                    sendhumidity("Humidity above 50%, switching OFF Humidifier.")
            if values[0] == "SmokeData":
                if float(values[1]) > 500.0:
                    sendsmoke()
        except :
            print("NOthing to do here..")

    db.commit()
    db.close()

def getremark(metric, value):
    return "OK"

def check_user_credentials(email, pwd):
    db = mysql.connector.connect(
        host="localhost",
        user="iotuser",
        password="iotpwd",
        database="iot315"
    )
    cursor = db.cursor()
    cursor.execute('SELECT COUNT(*) FROM Users WHERE email = %s AND password = %s', (email, pwd))
    row_count = cursor.fetchone()[0]
    cursor.close()
    db.close()

    return row_count == 1

def write_user(email, pwd):
    db = mysql.connector.connect(
        host="localhost",
        user="iotuser",
        password="iotpwd",
        database="iot315"
    )
    cursor = db.cursor()

    query = "INSERT INTO Users (email, password) VALUES (%s, %s)"
    values = (email, pwd)
    cursor.execute(query, values)

    db.commit()
    db.close()

def read_user(useremail, pwd):
    with open(usersfile, 'r') as file:
        for line in file:
            # Do something with the line
            list = line.strip()
            arr = [s.strip() for s in list.split(",")]
            if arr[0] == useremail and pwd == arr[1]:
                return True
        return False

def gettimestamp():
    timestamp = time.time()
    datetime_obj = datetime.datetime.fromtimestamp(timestamp)
    formatted_date = datetime_obj.strftime("%Y-%m-%d %H:%M:%S")
    return formatted_date

if __name__ == '__main__':
    app.run(debug=True)