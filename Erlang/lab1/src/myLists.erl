-module(myLists).


-export([contains/2, duplicateElements/1, sumFloats/1]).

contains([], N) -> false;
contains([H|T], H) -> true;
contains([_|T], N) -> contains(T, N).

duplicateElements([]) -> [];
duplicateElements([H|T]) -> [H, H | duplicateElements(T)].


sumFloats([]) -> 0;
sumFloats([H | T]) when is_float(H) ->
    H + sumFloats(T);
sumFloats([_|T])-> sumFloats(T).
