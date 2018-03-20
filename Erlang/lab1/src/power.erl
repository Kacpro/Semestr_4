-module(power).

-export([power/2]).

power(_, 0) -> 1;
power(N, E) -> N*power(N, E-1).
