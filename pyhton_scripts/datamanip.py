import csv

class DataParser:
	"""This class parses a csv file to extract some useful information"""

	# Total simulation time
	total_time = 0.0

	# Total kernel mode (Coala) execution time
	total_high_time = 0.0

	# Total user mode execution time
	total_low_time = 0.0

	# Schedule and commit execution time (kernel mode)
	sch_high_time = 0.0

	# Protected reads/writes execution time (kernel mode)
	mem_high_time = 0.0

	# Application execution time
	app_time = 0.0

	# Average schedule and commit execution time
	sch_avg = 0.0

	# Average protected reads/writes execution time
	mem_avg = 0.0

	# Average application execution time
	app_avg = 0.0

	# Schedule and commit load
	sch_dc = 0.0

	# Protected reads/writes load
	mem_dc = 0.0

	# User mode load
	usr_dc = 0.0

	# Number of schedule-commit executions
	sch_cnt = 0

	# Number of protected reads/writes executions
	mem_cnt = 0

	# Number of application executions
	app_cnt = 0

	def __init__(self, file, sch_ch, mem_ch, clk_freq, is_coala):

		# A GPIO set or clear is completed in 5 clock cycles for Coala (GCC),
		# and in 10 clock cycles for Alpaca (Clang)
		if is_coala:
			gpio_overhead = 5.0 / (clk_freq * 1000000)
		else:
			gpio_overhead = 10.0 / (clk_freq * 1000000)

		last_timestamp = 0.0
		# State
		sch_state = 0
		mem_state = 0
		app_state = 1
		# Last edge's timestamp
		app_lfe = 0.0
		sch_lre = 0.0
		mem_lre = 0.0

		with open(file, 'r') as csvfile:
			reader = csv.reader(csvfile)
			for tokens in reader:
		    	# Skip header line and negative timestamps
				if not 'Time' in tokens[0] and float(tokens[0]) >= 0:
					# Compute total simulation time
					interval = float(tokens[0]) - last_timestamp
					last_timestamp = float(tokens[0])
					self.total_time += interval - gpio_overhead
					# Compute number of full runs
					if app_state == 1:
						if int(tokens[1]) == 0:
							app_state = 0
							app_lfe = self.total_time
					else:
						if int(tokens[1]) == 1:
							app_state = 1
							self.app_time += self.total_time - app_lfe
							self.app_cnt += 1
					# Examine Scheduler channel
					# Search for rising edge
					if sch_state == 0:
						if int(tokens[sch_ch]) == 1:
							self.sch_cnt += 1
							sch_state = 1
							sch_lre = float(tokens[0])
					# Search for falling edge
					else:
						if int(tokens[sch_ch]) == 0:
							sch_state = 0
							high_interval = float(tokens[0]) - sch_lre
							self.sch_high_time += high_interval
					# Examine Memory Manager channel
					# Search for rising edge
					if mem_state == 0:
						if int(tokens[mem_ch]) == 1:
							self.mem_cnt += 1
							mem_state = 1
							mem_lre = float(tokens[0])
					# Search for falling edge
					else:
						if int(tokens[mem_ch]) == 0:
							mem_state = 0
							high_interval = float(tokens[0]) - mem_lre
							self.mem_high_time += high_interval

		sch_gpio_overhead = gpio_overhead * self.sch_cnt
		mem_gpio_overhead = gpio_overhead * self.mem_cnt
		self.sch_high_time -= sch_gpio_overhead
		self.mem_high_time -= mem_gpio_overhead
		self.total_high_time = self.sch_high_time + self.mem_high_time
		self.total_low_time = self.total_time - self.total_high_time

		# The scheduler channel is triggered twice per task: once for real
		# scheduling, and once inside the coala_next_task() macro
		if is_coala:
			self.sch_cnt = self.sch_cnt / 2

		if self.sch_cnt > 0:
			self.sch_avg = int(1000000 * self.sch_high_time / self.sch_cnt)
		if self.mem_cnt > 0:
			self.mem_avg = int(1000000 * self.mem_high_time / self.mem_cnt)
		if self.app_cnt > 0:
			self.app_avg = int(1000 * self.app_time / self.app_cnt)

		if self.total_time > 0:
			self.sch_dc = 100 * self.sch_high_time / self.total_time
			self.mem_dc = 100 * self.mem_high_time / self.total_time
			self.usr_dc = 100 * self.total_low_time / self.total_time