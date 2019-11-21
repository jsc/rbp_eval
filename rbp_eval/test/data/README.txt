Data for test runs.

The .qrels and .run files are qrels and run files, respectively.

The .spec files tie them together.  They have the following format:

Header:
    Line 1: the qrels file to use
    Line 2: the run file to use
    Line 3: the depths to use (a comma-separated list)
    Line 4: the p-values to use (a comma-separated list)
    Line 5: the ordering 

Body:

   Foreach query:
       num_rel
       num_ret
       Foreach depth:
          num_rel_ret
          Foreach p-value:
             <sum> <err>

   all_num_rel
   all_num_ret
   Foreach depth:
       all_num_rel_ret
       Foreach p-value:
           <avg-sum> <avg-err>

Lines starting with # are comments lines and are discarded

Sum and error values are calculated to 4 decimal places and are
considered to match if they are the same to 4 decimal places.
