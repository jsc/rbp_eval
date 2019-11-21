# As with 8.spec, but instead of scaling to fractional relevancies,
# we fold to binary relevance at 1
-B -d 2,5,10 -p 0.5,0.8,0.95 -s mult.qrels noties.run
#basic.qrels
#noties.run
#2,5,10
#0.5,0.8,0.95
#occurence
# qid=1
# num_rel
8.0
# num_ret
10.0
#   d=2
# num_rel_ret
1.0
# run-doc-1 is relevant, run-doc-2 is unjudged
# sum=(1 - p) * ((1 * p^0) + 0)
# err=accum_err + residual_err
# accum_err=(1 -p) * ((1 * p ^ 1))
# residual_err=p^2
# sum err
#     p=0.5
0.5000 0.5000
#     p=0.8
0.2000 0.8000
#     p=0.95
0.0500 0.9500
#   d=5
# run-doc relevances are (1, U, 0, U, 0)
# num_rel_ret
1.0
# sum=(1-p) * (p^0)
# residual_err=p^5
# accum_err=(1-p) * ((p^1) + (p^3))
#     p=0.5
0.5000 0.3438
#     p=0.8
0.2000 0.5901
#     p=0.95
0.0500 0.8641
#   d=10
# run-doc relevances are (1, U, 0, U, 0, 1, U, 0, U, 1)
# num_rel_ret
3.0
# sum=(1-p) * (p^0 + p^5 + p^9)
# residual_err=p^10
# accum_err=(1-p) * (p^1 + p^3 + p^6 + p^8)
#     p=0.5
0.5166 0.3232
#     p=0.8
0.2924 0.4558
#     p=0.95
0.1202 0.7590
# qid=2
# num_rel
9.0
# num_ret
10.0
#   d=2
# run-doc relevances are (U, 0)
# num_rel_ret
0.0
# sum=0
# residual_err=p^2
# accum_err=(1-p) * (p^0)
#    p=0.5
0.0000 0.7500
#    p=0.8
0.0000 0.8400
#    p=0.95
0.0000 0.9525
#   d=5
# run-doc relevances are (U, 0, 1, U, 1)
# num_rel_ret
2.0
# sum=(1-p) * (p^2 + p^4)
# residual_err=p^5
# accum_err=(1-p) * (p^0 + p^3)
#    p=0.5
0.1562 0.5938
#    p=0.8
0.2099 0.6301
#    p=0.95
0.0859 0.8666
#   d=10
# run-doc relevances are (U, 0, 1, U, 1, 1, 0, U, 1, U)
# num_rel_ret
4.0
# sum=(1-p) * (p^2 + p^4 + p^5 + p^8)
# residual_err=p^10
# accum_err=(1-p) * (p^0 + p^3 + p^7 + p^9)
#    p=0.5
0.1738 0.5684
#    p=0.8
0.3090 0.4786
#    p=0.95
0.1577 0.7580
# Averages
# num_rel
17.0
# num_ret
20.0
#  d=2
#  num_rel_ret
1.0
#    p=0.5
0.2500 0.6250
#    p=0.8
0.1000 0.8200
#    p=0.95
0.0250 0.9512
#  d=5
#  num_rel_ret
3.0
#    p=0.5
0.3281 0.4688
#    p=0.8
0.2050 0.6101
#    p=0.95
0.0679 0.8654
#  d=10
#  num_rel_ret
7.0
#    p=0.5
0.3452 0.4458
#    p=0.8
0.3007 0.4672
#    p=0.95
0.1390 0.7585
