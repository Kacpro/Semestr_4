
-module(qsort).

-export([qs/1, compareSpeeds/3, randomElems/3]).



lessThan(List, Arg) -> [X || X <- List, X < Arg].

grtEqThan(List, Arg) -> [X || X <- List, X >= Arg].

qs([Pivot|Tail]) -> qs(lessThan(Tail, Pivot)) ++ [Pivot] ++ qs(grtEqThan(Tail, Pivot)).



randomElems(N, Min, Max) -> [ rand:uniform(Max-Min)+Min || X <- lists:seq(1, N)].

compareSpeeds(List, Fun1, Fun2) ->
  {T1, _} = time:tc(Fun1(List)),
  {T2, _} = time:tc(Fun2(List)),
  {T1, T2}.


