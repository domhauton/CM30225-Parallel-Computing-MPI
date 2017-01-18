# CM30225-Parallel-Computing-Coursework-2
http://people.bath.ac.uk/masrjb/CourseNotes/cm30225.html

# Grade
90/100

# Report
Full LaTeX report is available at the link below.

https://github.com/domhauton/CM30225-Parallel-Computing-Coursework-2/blob/master/report/Report.pdf

# Running the Coursework
Scripts to run the coursework on Balena are available in **./bin** in the submitted coursework.
- **./bin/cw-prepare.sh** puts the compiled program in your ~. The source directory should be changed to match a clean version of the coursework.
- **./bin/cw-run.sh** will produce python jobs in your ~ and submit them using sbatch to balena. Outputs will appear in ~/out
- Running the **python3 ./bin/slurm_job_gen.py** will produce jobs for Balena in batches of 15 mins. 
- **./bin/cw-status.sh** Will display job status.
- **./bin/cw-collect-results.sh** will process the output of Balena and can be pasted directly into Google Sheets for analysis.

# Feedback

- mat_mpi_parity() a nice quick test for bit-for-bit inequality, but, of course, two matrices can have the same parity without being equal; and two matrices can both be relaxed to within a given precision without being equal
- Good to use Isend & Irecv, but you need to comment carefully on how you avoid data races on the buffers
- mat_scatter() Good to see MPI_Scatterv, but better it to avoid having all the matrix collected on one proc, as this will need a huge amount of memory for big matrices
- dispatcher_task_t: from reading the code alone, it is hard to discern your approach.  More (high level) comments in the code, please!
- Good use of MPI_Iallreduce()
- SWAR: nice, but not needed for this assignment
- Good code, but hugely over-engineered and more complicated than necessary!
- Correctness Testing:  OK. It would be worthwhile including in the writeup a list of sizes of matrices, numbers of procs and nodes that you tested on
- Figure 1. Good speedups
- Karp-Flatt: good. If you excluded the scatter/gather of the matrix, what would things look like?
- SWAR: nice, but not part of this assignment (MPI)
