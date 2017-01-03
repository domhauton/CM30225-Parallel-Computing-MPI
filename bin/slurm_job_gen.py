import os

directory = os.path.expanduser('~') + "/jobs"


def get_time_estimate(size, nodes):
    return ((((size - 2) * (size - 2) * 3000) / 65E6) / (nodes*0.7))+2


def get_job_executor(mpitasks, precision, size, job_type):
    return ["mpirun -np {:d} bin/parallel_computation_cw2 {:d} {:f} {:d}".format(mpitasks, size, precision, job_type)]


def gen_content(job_count, job_uid, nodes_required):
    return ["#!/bin/sh",
            "#SBATCH --account=cm30225",
            "#SBATCH --partition=teaching",
            "#SBATCH --job-name=cw2-{:02d}-{:02d}".format(job_uid, job_count),
            "#SBATCH --output=out/{:03d}-{:02d}-%j.out".format(job_uid, job_count),
            "#SBATCH --error=out/{:03d}-{:02d}-%j.err".format(job_uid, job_count),
            "",
            "#SBATCH --nodes={:d}".format(nodes_required),
            "#SBATCH --ntasks-per-node=16",
            "",
            "#SBATCH --time=00:15:00",
            "",
            "#SBATCH --mail-type=END",
            "",
            "#SBATCH --mail-user=dh499@bath.ac.uk",
            "# Run the programs"]


def gen_file_name(uid, count):
    return directory + "/cw2-{:03d}-{:02d}.slm".format(count, uid)


def write_to_file(job_list, job_uid, nodes_required):
    filename = gen_file_name(job_uid, len(job_list))
    print("Writing to: " + filename)
    f = open(filename, 'w')
    complete = gen_content(len(job_list), job_uid, nodes_required) + job_list
    f.write("\n".join(complete))
    f.close()


if not os.path.exists(directory):
    os.makedirs(directory)

currentPrecision = 0.0001
currentSize = 4096
nodesRequired = 4

jobQueue = []
jobQueue += get_job_executor(1, currentPrecision, currentSize, 0)

job_uid_ctr = 0
jobQueueTime = get_time_estimate(currentSize, 1)
jobQueueMaxTime = 60*15

for currentTasks in range(1, (16*nodesRequired)+1):
    job_uid_ctr += 1
    jobExecutor = get_job_executor(currentTasks, currentPrecision, currentSize, 1)
    timeEstimate = get_time_estimate(currentSize, currentTasks)
    if timeEstimate > jobQueueMaxTime:
        write_to_file(jobExecutor, job_uid_ctr, nodesRequired)
    elif timeEstimate + jobQueueTime > jobQueueMaxTime:
        write_to_file(jobQueue, job_uid_ctr, nodesRequired)
        jobQueue = jobExecutor
        jobQueueTime = timeEstimate
    else:
        jobQueue += jobExecutor
        jobQueueTime += jobQueueTime

if len(jobQueue) > 0:
    job_uid_ctr += 1
    write_to_file(jobQueue, job_uid_ctr, nodesRequired)
