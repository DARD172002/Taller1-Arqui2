import subprocess
import matplotlib.pyplot as plt

hw_threads = list(range(1, 12))# 1 a 11 hardware threads a usar

results_1_var = []
results_multi_var = []

def run_tls(hw_threads, shared_vars):
    percentage = []
    for threads in hw_threads:
        cmd = ["./tls_mod", str(threads), str(shared_vars)]
        try:
            output= subprocess.check_output(cmd).decode()
            lines =output.strip().split("\n")
            final = int([line for line in lines if "Final value" in line][0].split(":")[1].strip())
            success= int([line for line in lines if "Successful Commits" in line][0].split(":")[1].strip())
            max_commits=int([line for line in lines if "Theoretical maximum" in line][0].split(":")[1].strip())
            percentage.append((success / max_commits) * 100 if max_commits > 0 else 0)
        except Exception as e:
            print(f"Error  {threads} threads y {shared_vars} variable: {e}")
            percentage.append(0)
    return percentage

print("Corriendo con 1 variable compartida...")
results_1_var=run_tls(hw_threads, shared_vars=1)

print("Corriendo con varias variables compartidas...")
results_multi_var=run_tls(hw_threads, shared_vars=2)

# Graficar
plt.figure(figsize=(10, 6))
plt.plot(hw_threads, results_1_var, marker='o', label="successful commits (1 shared resource)", color='orange')
plt.plot(hw_threads, results_multi_var, marker='o', label="successful commits (more than one shared resource)", color='gray')
plt.title("percentage of successful commits vs HW threads\nfor different number of shared resources")
plt.xlabel("HW Threads")
plt.ylabel("Percentage of Successful Commits")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.show()
