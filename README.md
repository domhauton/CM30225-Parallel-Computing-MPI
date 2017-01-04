# CM30225-Parallel-Computing-Coursework-2
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
