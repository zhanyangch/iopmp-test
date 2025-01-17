import subprocess
import signal
import sys
import psutil
import os
import termios

# Example usage
output_file = "test_output.txt"  # File to save the output
qemu_folder = "~/qemu"
testcase_folder = "~/iopmp-test"
platform = "virt"

process = None
test_success = 0
test_cnt = 0

# Config (elf, additational command)
test_configs = [
    [f" {testcase_folder}/build/Test1.elf", f" --readconfig {testcase_folder}/qemu_cfg/{platform}/iopmp1-1.cfg"],
    [f" {testcase_folder}/build/Test1.elf", f" --readconfig {testcase_folder}/qemu_cfg/{platform}/iopmp1-2.cfg"],
    [f" {testcase_folder}/build/Test1.elf", f" --readconfig {testcase_folder}/qemu_cfg/{platform}/iopmp1-3.cfg"],
    [f" {testcase_folder}/build/Test2.elf", f" --readconfig {testcase_folder}/qemu_cfg/{platform}/iopmp1-1.cfg"],
    [f" {testcase_folder}/build/Test3.elf", f" --readconfig {testcase_folder}/qemu_cfg/{platform}/iopmp1-1.cfg"],
    [f" {testcase_folder}/build/Test4.elf", f" --readconfig {testcase_folder}/qemu_cfg/{platform}/iopmp4.cfg"],
    [f" {testcase_folder}/build/Test5.elf", f" --readconfig {testcase_folder}/qemu_cfg/{platform}/iopmp5-1.cfg"],
    [f" {testcase_folder}/build/Test5.elf", f" --readconfig {testcase_folder}/qemu_cfg/{platform}/iopmp5-2.cfg"],
    [f" {testcase_folder}/build/Test6.elf", f" --readconfig {testcase_folder}/qemu_cfg/{platform}/iopmp1-1.cfg"],
    [f" {testcase_folder}/build/Test6.elf", f" --readconfig {testcase_folder}/qemu_cfg/{platform}/iopmp1-2.cfg"],
    [f" {testcase_folder}/build/Test6.elf", f" --readconfig {testcase_folder}/qemu_cfg/{platform}/iopmp1-3.cfg"],
    [f" {testcase_folder}/build/Test7.elf", f" --readconfig {testcase_folder}/qemu_cfg/{platform}/iopmp7-1.cfg"],
    [f" {testcase_folder}/build/Test7.elf", f" --readconfig {testcase_folder}/qemu_cfg/{platform}/iopmp7-2.cfg"],
    [f" {testcase_folder}/build/Test8.elf", f" --readconfig {testcase_folder}/qemu_cfg/{platform}/iopmp8-1.cfg"],
    [f" {testcase_folder}/build/Test8.elf", f" --readconfig {testcase_folder}/qemu_cfg/{platform}/iopmp8-2.cfg"],
    [f" {testcase_folder}/build/Test9.elf", f" --readconfig {testcase_folder}/qemu_cfg/{platform}/iopmp9-1.cfg"],
    [f" {testcase_folder}/build/Test9.elf", f" --readconfig {testcase_folder}/qemu_cfg/{platform}/iopmp9-2.cfg"],
    [f" {testcase_folder}/build/Test10.elf", f" --readconfig {testcase_folder}/qemu_cfg/{platform}/iopmp10.cfg"],
    [f" {testcase_folder}/build/Test11.elf", f" --readconfig {testcase_folder}/qemu_cfg/{platform}/iopmp11.cfg"],
    [f" {testcase_folder}/build/Test12.elf", f" --readconfig {testcase_folder}/qemu_cfg/{platform}/iopmp12.cfg"],
    [f" {testcase_folder}/build/Test13.elf", f" --readconfig {testcase_folder}/qemu_cfg/{platform}/iopmp13.cfg"],
    [f" {testcase_folder}/build/Test14.elf", f" --readconfig {testcase_folder}/qemu_cfg/{platform}/iopmp14.cfg"],
    [f" {testcase_folder}/build/Test15.elf", f" --readconfig {testcase_folder}/qemu_cfg/{platform}/iopmp15.cfg"],
    [f" {testcase_folder}/build/Test16.elf", f" --readconfig {testcase_folder}/qemu_cfg/{platform}/iopmp16.cfg"],
    [f" {testcase_folder}/build/Test17.elf", f" --readconfig {testcase_folder}/qemu_cfg/{platform}/iopmp17.cfg"],
]

def kill_process(process):
    if process.poll() is None:
        pid = process.pid
        parent = psutil.Process(pid)
        for child in parent.children(recursive=True):
            # print(f"kill child pid {child.pid}")
            child.kill()
        parent.kill()
        # print(f"kill pid: {process.pid}")

def signal_handler(sig, frame):
    """Handle Ctrl-C to terminate processes gracefully."""
    print("\nCtrl-C received. Terminating processes...")
    global process
    kill_process(process)
    global original_settings
    # Restore terminal to its original settings.
    termios.tcsetattr(sys.stdin, termios.TCSADRAIN, original_settings)
    sys.exit(0)

def start_qemu(elf, cfg):
    command = f"{qemu_folder}/build/qemu-system-riscv64 -nographic -machine {platform},iopmp=true -cpu rv64,iopmp=true,iopmp_rrid=0 -bios {elf} {cfg}"
    global process
    try:
        print(f"Starting program: {command}")
        process = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    except Exception as e:
        print(f"Failed to start program: {command}. Error: {e}")
        return []
    # Open the file for writing
    with open(output_file, "a") as file:
        for line in process.stdout:
            line = line.strip()
            if line:
                print(f"Received: {line}")
                # Save the line to the file
                file.write(line + "\n")
                # Check for "success" or "failure"
                if "success" in line.lower():
                    print("Test passed! Found 'success' in the response.")
                    global test_success
                    test_success += 1
                    file.write("\n")
                    break
                elif "failure" in line.lower():
                    print("Test failed! Found 'failure' in the response.")
                    file.write("\n")
                    break
    kill_process(process)

# Save the original terminal settings
original_settings = termios.tcgetattr(sys.stdin)

# Remove old output file
try:
    os.remove(output_file)
    print(f"File {output_file} has been removed.")
except FileNotFoundError:
    print(f"File {output_file} not found.")
except PermissionError:
    print(f"Permission denied to delete {output_file}.")
except Exception as e:
    print(f"Error occurred: {e}")

# Start testcases
for test_cfg in test_configs:
    test_cnt += 1
    signal.signal(signal.SIGINT, lambda sig, frame: signal_handler(sig, frame))
    start_qemu(test_cfg[0], test_cfg[1])

print(f"test pass: {test_success} / {test_cnt}")
# Restore terminal to its original settings.
termios.tcsetattr(sys.stdin, termios.TCSADRAIN, original_settings)