-module(onp).

-export([onp/1]).

onp(Str) ->
  Tokens = string:tokens(str, " "),
  Stack = for(Tokens).




