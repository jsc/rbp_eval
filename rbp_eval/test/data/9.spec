# As with 7.spec, but we use a qrels file with rels of 1, 2, and 4, and
# then down-scale them to the range 0.25 to 1.0
-f 0.25 -d 2,5,10 -p 0.5,0.8,0.95 -s mult.qrels noties.run
#frac.qrels
#noties.run
#2,5,10
#0.5,0.8,0.95
#score
# qid=1
# run-doc relevances are (0.5, U, 0, U, 0, 0.25, U, 0, U, 1.0)
# num_rel
5.0
# num_ret
10.0
#   d=2
# num_rel_ret
0.5
# sum err
#     p=0.5
0.2500 0.5000
#     p=0.8
0.1000 0.8000
#     p=0.95
0.0250 0.9500
#   d=5
# num_rel_ret
0.5
#     p=0.5
0.2500 0.3438
#     p=0.8
0.1000 0.5901
#     p=0.95
0.0250 0.8641
#   d=10
# num_rel_ret
1.75
#     p=0.5
0.2549 0.3232
#     p=0.8
0.1432 0.4558
#     p=0.95
0.0662 0.7590
# qid=2
# run-doc relevances are  (U, 0, 0.5, U, 1.0, 0.25, 0, U, 0.25, U)
# num_rel
5.25
# num_ret
10.0
#   d=2
# num_rel_ret
0.0
#    p=0.5
0.0000 0.7500
#    p=0.8
0.0000 0.8400
#    p=0.95
0.0000 0.9525
#   d=5
# num_rel_ret
1.5
#    p=0.5
0.0938 0.5938
#    p=0.8
0.1459 0.6301
#    p=0.95
0.0633 0.8666
#   d=10
# num_rel_ret
2.0
#    p=0.5
0.0981 0.5684
#    p=0.8
0.1707 0.4786
#    p=0.95
0.0813 0.7580
# Averages
# num_rel
10.25
# num_ret
20.0
#  d=2
#  num_rel_ret
0.5
#    p=0.5
0.1250 0.6250
#    p=0.8
0.0500 0.8200
#    p=0.95
0.0125 0.9512
#  d=5
#  num_rel_ret
2.0
#    p=0.5
0.1719 0.4688
#    p=0.8
0.1230 0.6101
#    p=0.95
0.0441 0.8654
#  d=10
#  num_rel_ret
3.75
#    p=0.5
0.1765 0.4458
#    p=0.8
0.1570 0.4672
#    p=0.95
0.0737 0.7585
