# similar to 5.spec, except that the results are jumbled
-d 2,5,10 -p 0.5,0.8,0.95 basic.qrels ties-jumbled.run
#basic.qrels
#ties.run
#2,5,10
#0.5,0.8,0.95
#rank
# qid=1
# num_rel
8.0
# num_ret
10.0
# run-doc relevances are (1, (U, 0), U, (0, 1), U, 0, (U, 1)
#   d=2
# num_rel_ret
1.0
# sum err
#     p=0.5
0.5000 0.3438
#     p=0.8
0.2000 0.7120
#     p=0.95
0.0500 0.9257
#   d=5
# num_rel_ret
1.5
#     p=0.5
0.5117 0.2812
#     p=0.8
0.2369 0.5741
#     p=0.95
0.0699 0.8630
#   d=10
# num_rel_ret
3.0
#     p=0.5
0.5249 0.2603
#     p=0.8
0.3039 0.4364
#     p=0.95
0.1220 0.7570
# qid=2
# num_rel
9.0
# num_ret
11.0
# run-doc relevances are ((U, 0), 1, U, 1, 1, 0, U, 1, (U, 0) ) 
#   d=2
# num_rel_ret
0.0
#    p=0.5
0.0000 0.6250
#    p=0.8
0.0000 0.8200
#    p=0.95
0.0000 0.9513
#   d=5
# num_rel_ret
2.0
#    p=0.5
0.1562 0.4688
#    p=0.8
0.2099 0.6101
#    p=0.95
0.0859 0.8654
#   d=10
# num_rel_ret
4.0
#    p=0.5
0.1738 0.4427
#    p=0.8
0.3090 0.4438
#    p=0.95
0.1577 0.7406
# Averages
# num_rel
17.0
# num_ret
21.0
#  d=2
# num_rel_ret
1.0
#    p=0.5
0.2500 0.484375
#    p=0.8
0.1000 0.766000
#    p=0.95
0.0250 0.938453
#  d=5
# num_rel_ret
3.5
#    p=0.5
0.3340 0.375000
#    p=0.8
0.2234 0.592080
#    p=0.95
0.0779 0.864181
#  d=10
# num_rel_ret
7.0
#    p=0.5
0.3494 0.351501
#    p=0.8
0.3065 0.440099
#    p=0.95
0.1399 0.748825
