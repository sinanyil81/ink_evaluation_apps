import sys
import datamanip as dm

import numpy as np
import matplotlib.pyplot as plt

clk_freq = 1

data = dm.DataParser("adxl35_mic.csv", 4, 5, 1, 2, 0.20)

# print("Total:%d, Correct:%d, False:%d"%(data.counted_events,data.counted_corr,data.counted_incorr))
print("")
print("=========================================================================================")
print("  User  |    Scheduling    		   |   DMA buffer			   |  Application  ")
print(" ------ | ------------------------- | ------------------------- | --------------")
print(" % load | % load | avg[us] |   cnt  | % load | avg[us] |   cnt  | runs | avg[ms]")
print("-----------------------------------------------------------------------------------------")

tab_line = ""
tab_line += "%6.3f" % data.usr_dc + " | " + \
            "%6.3f" % data.sch_dc + " | " + \
            "%7d" % data.sch_avg + " | " + \
            "%6d" % data.sch_cnt + " | " + \
            "%6.3f" % data.mem_dc + " | " + \
            "%7d" % data.mem_avg + " | " + \
            "%6d" % data.mem_cnt + " | " + \
            "%4d" % data.app_cnt + " | " \
            "%7d" % data.app_avg

print(tab_line)


# # Example 1 -- Mostly defaults
# # This demonstrates how to create a simple diagram by implicitly calling the
# # Sankey.add() method and by appending finish() to the call to the class.
# Sankey(flows=[0.25, 0.15, 0.60, -0.20, -0.15, -0.05, -0.50, -0.10],
#        labels=['', '', '', 'First', 'Second', 'Third', 'Fourth', 'Fifth'],
#        orientations=[-1, 1, 0, 1, 1, 1, 0, -1]).finish()
# plt.title("The default settings produce a diagram like this.")
# # Notice:
# #   1. Axes weren't provided when Sankey() was instantiated, so they were
# #      created automatically.
# #   2. The scale argument wasn't necessary since the data was already
# #      normalized.
# #   3. By default, the lengths of the paths are justified.

# # Example 2
# # This demonstrates:
# #   1. Setting one path longer than the others
# #   2. Placing a label in the middle of the diagram
# #   3. Using the scale argument to normalize the flows
# #   4. Implicitly passing keyword arguments to PathPatch()
# #   5. Changing the angle of the arrow heads
# #   6. Changing the offset between the tips of the paths and their labels
# #   7. Formatting the numbers in the path labels and the associated unit
# #   8. Changing the appearance of the patch and the labels after the figure is
# #      created
# fig = plt.figure()
# ax = fig.add_subplot(1, 1, 1, xticks=[], yticks=[],
#                      title="Flow Diagram of an Application")
# sankey = Sankey(ax=ax, scale=0.01, offset=0.2, head_angle=180,
#                 format='%.0f', unit='%')
# sankey.add(flows=[data.app_avg, -data.mem_avg, -data.sch_avg],
#            labels=['Execution Time', 'PerMemory Time', 'Scheduling Time'],
#            orientations=[0, 0, 0],
#            pathlengths=[1, 0.25, 0.25],
#            patchlabel="Time(ms)\nA")  # Arguments to matplotlib.patches.PathPatch()
# diagrams = sankey.finish()
# diagrams[0].texts[-1].set_color('r')
# diagrams[0].text.set_fontweight('bold')
# # Notice:
# #   1. Since the sum of the flows is nonzero, the width of the trunk isn't
# #      uniform.  If verbose.level is helpful (in matplotlibrc), a message is
# #      given in the terminal window.
# #   2. The second flow doesn't appear because its value is zero.  Again, if
# #      verbose.level is helpful, a message is given in the terminal window.
# plt.show()
