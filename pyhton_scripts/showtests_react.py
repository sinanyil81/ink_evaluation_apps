import sys
import reactivity_stats as dm

import numpy as np
import matplotlib.pyplot as plt

#def __init__(self, file, env_st_ch, high_prio_tsk_ch, env_evnt_ch, low_prio_tsk_ch, sens_st_ch,sens_evnt_ch, time_limit, ):
# env_st_ch: High priority interrupt channel
env_st_ch 			= 9
# high_prio_tsk_ch: High priority task execution channel
high_prio_tsk_ch 	= 1
# env_evnt_ch: Low priority interrupt channel
env_evnt_ch 		= 8
# low_prio_tsk_ch: Low priority task execution channel
low_prio_tsk_ch 	= 7
# sens_st_ch: completion high priority task finish channel
sens_st_ch 			= 0
# sens_evnt_ch: completion low priority task finish channel
sens_evnt_ch		= 3
# time_limit: expiration time of event recognition
time_limit 			= 0.75
# sleep channel 
sleep_ch			= 10

data = dm.DataParser("alpaca_180s_power_test_T_state_machine.csv", 
	env_st_ch + 1,
 	high_prio_tsk_ch + 1,
	env_evnt_ch + 1, 
	low_prio_tsk_ch + 1,
	sens_st_ch + 1,
	sens_evnt_ch + 1,
	time_limit,
	sleep_ch + 1)

print("")
print("/=============STATS 4 NERDS============/")
print("")
print("Time limit for successful capture:%fs"%(time_limit))
print("")
print("Total events:%d"%(data.counted_events))
print("|_Successfully Captured:%d"%(data.counted_corr))
print("|__Falsely Captured:%d"%(data.counted_incorr))
print("|___Missed:%d"%(data.counted_events -(data.counted_corr + data.counted_incorr)))
print("")
print("Success Rate:	%f"%(float(data.counted_corr)/float(data.counted_events)))
print("")
print("False Rate: 	%f"%(float(data.counted_incorr)/float(data.counted_events)))
print("")
print("Miss Rate: 	%f"%(float(data.counted_events - (data.counted_corr + data.counted_incorr))/float(data.counted_events)))
print("")
print("+++++++++++++++++++++++++++++++++++++++")
print("")
print("Total Time:%6.3fs"%(data.total_time))
print("|_High priority execution:%6.3fs (avg/tsk:%6.3fs)"%(data.hp_time,data.hp_avg_time))
print("|__Low priority execution :%6.3fs (avg/tsk:%6.3fs)"%(data.lp_time,data.lp_avg_time))
print("|___Sleep time :%6.3fs (avg/instance:%6.3fs)"%(data.sleep_time,data.avg_sleep_time))
print("|____Remaining :%6.3fs"%(data.total_time - data.hp_time - data.lp_time - data.sleep_time))
print("")
print("|Avg State change reaction time(high priority):%6.5fs"%(data.state_reaction_time))
print("|Avg Event change reaction time(low priority):%6.5fs"%(data.event_reaction_time))
print("")
print("/=============END====================/")

# print("")
# print("=========================================================================================")
# print("  User  |    Scheduling    		   |   DMA buffer			   |  Application  ")
# print(" ------ | ------------------------- | ------------------------- | --------------")
# print(" % load | % load | avg[us] |   cnt  | % load | avg[us] |   cnt  | runs | avg[ms]")
# print("-----------------------------------------------------------------------------------------")

# tab_line = ""
# tab_line += "%6.3f" % data.usr_dc + " | " + \
#             "%6.3f" % data.sch_dc + " | " + \
#             "%7d" % data.sch_avg + " | " + \
#             "%6d" % data.sch_cnt + " | " + \
#             "%6.3f" % data.mem_dc + " | " + \
#             "%7d" % data.mem_avg + " | " + \
#             "%6d" % data.mem_cnt + " | " + \
#             "%4d" % data.app_cnt + " | " \
#             "%7d" % data.app_avg

# print(tab_line)


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
