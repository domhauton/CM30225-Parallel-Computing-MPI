# CM30225-Parallel-Computing-Coursework-1
http://people.bath.ac.uk/masrjb/CourseNotes/cm30225.html

# Grade
Unknown

# Running the Coursework
Scripts to run the coursework on Balena are available in **./bin** in the submitted coursework.
- **./bin/cw-prepare.sh** puts the compiled program in your ~. The source directory should be changed to match a clean version of the coursework.
- **./bin/cw-run.sh** will produce python jobs in your ~ and submit them using sbatch to balena. Outputs will appear in ~/out
- Running the **python3 ./bin/slurm_job_gen.py** will produce jobs for Balena in batches of 15 mins. 
- **./bin/cw-status.sh** Will display job status.
- **./bin/cw-collect-results.sh** will process the output of Balena and can be pasted directly into Google Sheets for analysis.

# Code Structure
The code is separated into multiple files and uses pointer obfuscation, to structure the program.
- *mat.c* and the *mat_t* is the core of the code and represents the matrix that will be relaxed.
- *mat.c* holds helper functions that operate on *mat_t* and also contains the sequential version of the smoothing algorithm.
- *mat_itr_t* and *mat_itr_edge_t* are iterators used to parse the matrix.
- mat_itr_t can also be  split into two new iterators. The two iterators will now step twice as far in each iteration out of sync. This can be done recursively until you get the size of iterator desired.
- *spool.c* (smoothing pool) is used to process tasks given 
