import onnx

m = onnx.load("models/driver_classifier.onnx")

for i in m.graph.input:
    print("INPUT:", i.name)

for o in m.graph.output:
    print("OUTPUT:", o.name)
    