from flask import Flask

app = Flask(__name__)
app.route('/')
def index():
    return "Hellow, Code Analysis"

#if __name__=="__main__":
if __name__ == "__main__":
    app.run()
