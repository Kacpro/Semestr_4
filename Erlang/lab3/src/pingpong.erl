-module(pingpong).

-export([start/0, stop/0, play/1, ping/0, pong/0]).



start() ->
  register(ping, spawn(?MODULE, ping, [])),
  register(pong, spawn(?MODULE, pong, [])).



stop() ->
  ping ! stop,
  pong ! stop.



play(N) ->
  ping ! N.


ping() ->
  receive
    stop -> ok;
    N ->
      io:fwrite("ping ~.10B~n", [N]),
      timer:sleep(1000),
      pong ! N-1,
      ping()
  after
    20000 -> ok
  end.

d17-l427-pc-17.ipa.iisg.agh.edu.pl

pong() ->
  receive
    stop -> ok;
    N ->
      io:fwrite("pong ~.10B~n", [N]),
      timer:sleep(1000),
      ping ! N-1,
      pong()
  after
    20000 -> ok
  end.