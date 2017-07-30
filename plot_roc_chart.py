import numpy as np
import matplotlib.pyplot as plt
import sys 

CSV_FILE = "classifier_result.csv"
with open(CSV_FILE) as f:
    data = f.read().strip().split()

INFO = ""
if len(sys.argv) > 1:
    INFO = sys.argv[1]

results = {}
for a_data in data:
    a_data_splitted = a_data.split(',')
    condition = ','.join(a_data_splitted[:3])
    TP, TN, FP, FN = map(lambda x: float(x), a_data_splitted[3:])
    if condition in results:
        TP0, TN0, FP0, FN0 = results[condition]
        TP += TP0 
        TN += TN0 
        FP += FP0
        FN += FN0
    results[condition] = TP, TN, FP, FN

test_key = "12,3.80,0.50"
if test_key in results:
    print(test_key, results[test_key])

plot_data = {"x": {"x": [], "y": []}, "O": {"x": [], "y": []}, "o": {"x": [], "y": []}}
for key in results.keys():
    threshold, ND_x, ND_s = key.split(',')
    threshold = int(threshold)
    TP, TN, FP, FN = results[key]
    y = TP / (TP + FN)
    x = FP / (FP + TN)
    marker = "O"
    label = "$8 \leq threshold < 20$"
    if threshold < 8:
        label = "$threshold < 8$"
        marker = "x"
    elif threshold >= 20:
        label = "$20 \leq threshold$"
        marker = "o"
    plot_data[marker]["x"].append(x)
    plot_data[marker]["y"].append(y)
    plot_data[marker]["label"] = label

for marker in plot_data.keys():
    x = plot_data[marker]["x"]
    y = plot_data[marker]["y"]
    label = plot_data[marker]["label"]
    if marker == 'O':
        marker = 'o'
        facecolors = 'none'
    else:
        facecolors = 'k'
    plt.scatter(x, y, label=label, color="k", facecolors=facecolors, s=15, marker=marker, lw=0.75)
IMG_FILE = CSV_FILE.replace(".csv", "_%s.png" % INFO)
plt.title("ROC (mode=%s)" % INFO)
plt.minorticks_on()
plt.axis((0.0, 1.0, 0.0, 1.0))
plt.xlabel("False Positive Rate")
plt.ylabel("True Positive Rate")
plt.gca().set_aspect('equal', adjustable='box') # square plot area
leg = plt.legend(fancybox=False, edgecolor='k', shadow=False, loc='right') # Hanrei
leg.get_frame().set_linewidth(0.7)
plt.savefig(IMG_FILE, bbox_inches='tight', dpi=350)