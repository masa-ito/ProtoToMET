import numpy as np
import matplotlib.pyplot as plt

# Create a figure of size 8x6 inches, 80 dots per inch
plt.figure(figsize=(8, 6), dpi=80)

# Create a new subplot from a grid of 1x1
plt.subplot(1, 1, 1)

threadNums = [ 1, 2, 4, 
               8, 12, 16, 
               20, 24];

# elasped time (milisecond) for conjugate gradient
elapsedNonMeta = [ 3000.91, 1560.39, 832.975, 
                   507.925, 430.125, 451.413, 
                   392.125, 434.75] 

elapsedMeta = [ 2929.45, 1544.96,  838.275,
                515.638, 432.825, 402.375,
                407.012, 459.425] 

# the num. of doing conjugate gradient method in a second
speedNonMeta = [ 1000.0 / t for t in elapsedNonMeta ] 
speedMeta = [ 1000.0 / t for t in elapsedMeta ] 

#plt.plot(X, C, color="blue", linewidth=1.0, linestyle="-")
plt.plot(threadNums, speedMeta, 
         color="blue", linewidth=3.0, linestyle="-",
         marker="o", markersize=10.0,
         label="EDSL")
plt.plot(threadNums, speedNonMeta, 
         color="red", linewidth=3.0, linestyle="--",
         marker="^", markersize=10.0,
         label="Non-OOP")

ax = plt.gca()
for label in ax.get_xticklabels() + ax.get_yticklabels():
    label.set_fontsize(20)
    #label.set_bbox(dict(facecolor='white', edgecolor='None', alpha=0.65))

plt.legend(loc='lower right', fontsize=24)

# Set x limits
#plt.xlim(0.0, 24.0)

# Set x ticks
#plt.xticks([0, 4, 8, 16, 20, 24])

# Set y limits
#plt.ylim(0.000, 0.003)

# Set y ticks
#plt.yticks([0.000, 0.003, 0.0005])

# Save figure using 72 dots per inch
plt.savefig("benchmarkPlot20160529.png", dpi=200)
plt.savefig("benchmarkPlot20160529.jpg", dpi=200)

# Show result on screen
plt.show()
