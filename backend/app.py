from flask import Flask, render_template, request, jsonify
from datetime import datetime
from collections import deque

app = Flask(__name__)

history = deque(maxlen=40)
alerts = deque(maxlen=30)
last_alert_time = datetime.now()


@app.route('/')
def index():
    return render_template('index.html')


@app.route('/update', methods=['POST'])
def update():
    global last_alert_time

    data = request.get_json()
    now_obj = datetime.now()
    now_str = now_obj.strftime("%H:%M:%S")

    data['time'] = now_str

    # Alert logic
    if (now_obj - last_alert_time).total_seconds() > 3:
        t = float(data['temp'])
        b = int(data['bpm'])
        s = int(data['spo2'])

        problems = []

        if t > 38.5:
            problems.append("High Temp")
        if b > 120:
            problems.append("High Heart Rate")
        if s < 92:
            problems.append("Low Oxygen")

        if problems:
            msg = " & ".join(problems) + " Observed"
            alerts.appendleft({
                "time": now_str,
                "msg": msg,
                "type": "critical"
            })
            last_alert_time = now_obj

    history.appendleft(data)

    return jsonify({"status": "success"})


@app.route('/get_data')
def get_data():
    return jsonify({
        "history": list(history),
        "alerts": list(alerts)
    })


if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)
