################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
appinit.obj: ../appinit.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"/opt/ti/ccsv7/tools/compiler/ti-cgt-msp430_17.9.0.STS/bin/cl430" -vmspx --data_model=restricted --use_hw_mpy=F5 --include_path="/opt/ti/ccsv7/ccs_base/msp430/include" --include_path="/home/dpatoukas/Documents/TUDelft/Thesis/Ink++/InK/mcu/msp430" --include_path="/home/dpatoukas/Documents/TUDelft/Thesis/Ink++/InK/isr" --include_path="/home/dpatoukas/Documents/TUDelft/Thesis/Ink++/InK/scheduler" --include_path="/home/dpatoukas/Documents/TUDelft/Thesis/Ink++/InK/mcu" --include_path="/home/dpatoukas/Documents/TUDelft/Thesis/Ink++/InK" --include_path="/home/dpatoukas/Documents/TUDelft/Thesis/Ink++/InKApplications/cm_acc" --include_path="/opt/ti/ccsv7/tools/compiler/ti-cgt-msp430_17.9.0.STS/include" --advice:power=all --advice:hw_config=all --define=__MSP430FR5969__ --define=_MPU_ENABLE -g --printf_support=minimal --diag_warning=225 --diag_wrap=off --display_error_number --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU40 --preproc_with_compile --preproc_dependency="appinit.d_raw" $(GEN_OPTS__FLAG) "$(shell echo $<)"
	@echo 'Finished building: $<'
	@echo ' '

thread1.obj: ../thread1.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"/opt/ti/ccsv7/tools/compiler/ti-cgt-msp430_17.9.0.STS/bin/cl430" -vmspx --data_model=restricted --use_hw_mpy=F5 --include_path="/opt/ti/ccsv7/ccs_base/msp430/include" --include_path="/home/dpatoukas/Documents/TUDelft/Thesis/Ink++/InK/mcu/msp430" --include_path="/home/dpatoukas/Documents/TUDelft/Thesis/Ink++/InK/isr" --include_path="/home/dpatoukas/Documents/TUDelft/Thesis/Ink++/InK/scheduler" --include_path="/home/dpatoukas/Documents/TUDelft/Thesis/Ink++/InK/mcu" --include_path="/home/dpatoukas/Documents/TUDelft/Thesis/Ink++/InK" --include_path="/home/dpatoukas/Documents/TUDelft/Thesis/Ink++/InKApplications/cm_acc" --include_path="/opt/ti/ccsv7/tools/compiler/ti-cgt-msp430_17.9.0.STS/include" --advice:power=all --advice:hw_config=all --define=__MSP430FR5969__ --define=_MPU_ENABLE -g --printf_support=minimal --diag_warning=225 --diag_wrap=off --display_error_number --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU40 --preproc_with_compile --preproc_dependency="thread1.d_raw" $(GEN_OPTS__FLAG) "$(shell echo $<)"
	@echo 'Finished building: $<'
	@echo ' '

thread2.obj: ../thread2.c $(GEN_OPTS) | $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"/opt/ti/ccsv7/tools/compiler/ti-cgt-msp430_17.9.0.STS/bin/cl430" -vmspx --data_model=restricted --use_hw_mpy=F5 --include_path="/opt/ti/ccsv7/ccs_base/msp430/include" --include_path="/home/dpatoukas/Documents/TUDelft/Thesis/Ink++/InK/mcu/msp430" --include_path="/home/dpatoukas/Documents/TUDelft/Thesis/Ink++/InK/isr" --include_path="/home/dpatoukas/Documents/TUDelft/Thesis/Ink++/InK/scheduler" --include_path="/home/dpatoukas/Documents/TUDelft/Thesis/Ink++/InK/mcu" --include_path="/home/dpatoukas/Documents/TUDelft/Thesis/Ink++/InK" --include_path="/home/dpatoukas/Documents/TUDelft/Thesis/Ink++/InKApplications/cm_acc" --include_path="/opt/ti/ccsv7/tools/compiler/ti-cgt-msp430_17.9.0.STS/include" --advice:power=all --advice:hw_config=all --define=__MSP430FR5969__ --define=_MPU_ENABLE -g --printf_support=minimal --diag_warning=225 --diag_wrap=off --display_error_number --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU40 --preproc_with_compile --preproc_dependency="thread2.d_raw" $(GEN_OPTS__FLAG) "$(shell echo $<)"
	@echo 'Finished building: $<'
	@echo ' '


