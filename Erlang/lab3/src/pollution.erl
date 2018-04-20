
-module(pollution).

-export([addStation/2, addValue/4, removeValue/3, getOneValue/3, getStationMean/2, getDailyMean/2, getMinimumPollutionStation/1, start/0, stop/0]).

-record(measurement, {time, type, value}).



createMonitor() -> #{}.



addStation(Name, Location, Monitor) ->
  case lists:any(fun ({Nam,{Loc, _}}) -> ((Nam == Name) or (Loc == Location)) end, maps:to_list(Monitor))  of
    true -> "Station with the same name or location exists";
    _ -> Monitor#{Name => {Location, []}}
  end.

getName(Location, Monitor) ->
  M = maps:filter(fun (_, {L,_}) -> L == Location end, Monitor),
  case maps:to_list(M) of
    [{Key, _}] -> Key;
    [{}] -> ""
  end.



addValue({X,Y}, Time, Type, Val, Monitor) ->
  Value = float(Val),
  K = getName({X,Y}, Monitor),
  case K of
    "" -> "No such station";
    Key ->
      {_, List} = maps:get(Key, Monitor),
      case lists:any(fun (M) -> (M#measurement.time == Time) and (M#measurement.type == Type) end, List) of
        true -> "This measurement already exists";
        _ -> Monitor#{Key => {{X,Y}, [ #measurement{time = Time, type = Type, value = Value} | List]}}
      end
  end;

addValue(Name, Time, Type, Val, Monitor) ->
  Value = float(Val),
  case maps:get(Name, Monitor, error) of
    error -> "No such station";
    {Location, List} ->
      case lists:any(fun (M) -> (M#measurement.time == Time) and (M#measurement.type == Type) end, List) of
        true -> "This measurement already exists";
        _ -> Monitor#{Name => {Location, [ #measurement{time = Time, type = Type, value = Value} | List]}}
      end
  end.



removeValue({X,Y}, Time, Type, Monitor) ->
  K = getName({X,Y}, Monitor),
  case K of
    "" -> "No such station";
    Key ->
      {_, List} = maps:get(Key, Monitor),
      Monitor#{Key => {{X,Y}, lists:filter(fun (M) -> (M#measurement.type /= Type) or (M#measurement.time /= Time) end, List)}}
  end;

removeValue(Name, Time, Type, Monitor) ->
  case maps:get(Name, Monitor, error) of
    error -> "No such station";
    {Location, List} ->
      Monitor#{Name => {Location, lists:filter(fun (M) -> (M#measurement.type /= Type) or (M#measurement.time /= Time) end, List)}}
  end.



getOneValue({X,Y}, Time, Type, Monitor) ->
  Key = getName({X,Y}, Monitor),
  case maps:get(Key, Monitor, error) of
    error -> "No such station";
    {_, List} -> case lists:filter(fun (M) -> (M#measurement.type == Type) and (M#measurement.time == Time) end, List) of
                   [] -> "No such measurement";
                   [#measurement{value = Value}] -> Value
                  end
end;

getOneValue(Name, Time, Type, Monitor) ->
  case maps:get(Name, Monitor, error) of
    error -> "No such station";
    {_, List} -> case lists:filter(fun (M) -> (M#measurement.type == Type) and (M#measurement.time == Time) end, List) of
                   [] -> "No such measurement";
                   [#measurement{value = Value}] -> Value
                 end
  end.



getStationMean({X,Y}, Type, Monitor) ->
  Key = getName({X,Y}, Monitor),
  case maps:get(Key, Monitor, error) of
    error -> "No such station";
    {_, List} ->
      case lists:filter(fun (M) -> M#measurement.type == Type end, List) of
        [] -> "No such measurement";
        List1 -> lists:foldr(fun (M, Acc) -> M#measurement.value+Acc end, 0, List1)/(erlang:length(List1))
      end
  end;

getStationMean({X,Y}, Type, Monitor) ->
  Key = getName({X,Y}, Monitor),
  case maps:get(Key, Monitor, error) of
    error -> "No such station";
    {_, List} ->
      case lists:filter(fun (M) -> M#measurement.type == Type end, List) of
        [] -> "No such measurement";
        List1 -> lists:foldr(fun (M, Acc) -> M#measurement.value+Acc end, 0, List1)/(erlang:length(List1))
      end
  end.



getDailyMean(Time, Type, Monitor) ->
  List1 = lists:map(fun ({_,{_, L}}) -> L end, maps:to_list(Monitor)),
  case lists:filter(fun (M) -> (M#measurement.type == Type)
                                and ( case M#measurement.time of
                                        {Time, _} -> true;
                                        _ -> false
                                      end )
                    end, lists:flatten(List1)) of
    [] -> 0;
    List -> lists:foldr(fun (M, Acc) -> M#measurement.value+Acc end, 0, List)/(erlang:length(List))
  end.



head([H|_]) -> H.



getMinimumPollutionStation(Type, Monitor) ->
  List = lists:map(fun ({N,{_, L}}) -> {N,L} end, maps:to_list(Monitor)),
  List1 = lists:map(fun ({N,L}) -> {N, head(L)} end, lists:filter(fun ({_,L}) -> L /= [] end, lists:map(fun ({N,L}) -> {N, (lists:filter(fun (M) -> M#measurement.type == Type end, L))} end,List))),
  case List1 of
    [] -> "No enough data";
    _ -> {Name, _} = lists:foldr(fun ({N,V}, {N1, V1}) -> case V<V1 of
                                                            true -> {N,V};
                                                            _ -> {N1, V1}
                                                          end
                                 end, head(List1) ,List1),
      Name
end.



start() ->
  register(server, spawn(?MODULE, init, [])).

init() ->
  Monitor = createMonitor(),
  loop(Monitor).

loop(Monitor) ->
  receive
    {msg, Pid, addS, Name, Location} -> Pid ! ok, loop(addStation(Name, Location, Monitor));
    {msg, Pid, addV, {X,Y}, Time, Type, Val} -> Pid ! ok, loop(addValue({X,Y}, Time, Type, Val, Monitor));
    {msg, Pid, addV, Name, Time, Type, Val} -> Pid ! ok, loop(addValue(Name, Time, Type, Val, Monitor));
    {msg, Pid, rmV, {X,Y}, Time, Type} -> Pid ! ok, loop(removeValue({X,Y}, Time, Type, Monitor));
    {msg, Pid, rmV, Name, Time, Type} -> Pid ! ok, loop(removeValue(Name, Time, Type, Monitor));
    {msg, Pid, get1V, {X,Y}, Time, Type} -> Pid ! getOneValue({X,Y}, Time, Type, Monitor), loop(Monitor);
    {msg, Pid, get1V, Name, Time, Type} -> Pid ! getOneValue(Name, Time, Type, Monitor), loop(Monitor);
    {msg, Pid, getSMean, {X,Y}, Type} -> Pid ! getStationMean({X,Y}, Type, Monitor), loop(Monitor);
    {msg, Pid, getSMean, Name, Type} -> Pid ! getStationMean(Name, Type, Monitor), loop(Monitor);
    {msg, Pid, getDMean, Time, Type} -> Pid ! getDailyMean(Time, Type, Monitor), loop(Monitor);
    {msg, Pid, getMinS, Type} -> Pid ! getMinimumPollutionStation(Type, Monitor), loop(Monitor);

    stop -> ok
  end.


stop() ->
  server ! stop.



addStation(Name, Location) ->
  server ! {msg, self(), addS, Name, Location},
  receive
    Result -> Result
  end,
  io:format("~p~n", [Result]).



addValue({X,Y}, Time, Type, Val) ->
  server ! {msg, self(), addV, {X,Y}, Time, Type, Val},
  receive
    Result -> Result
  end,
  io:format("~p~n", [Result]);
addValue(Name, Time, Type, Val) ->
  server ! {msg, self(), addV, Name, Time, Type, Val},
  receive
    Result -> Result
  end,
  io:format("~p~n", [Result]).



removeValue({X,Y}, Time, Type) ->
  server ! {msg, self(), rmV, {X,Y}, Time, Type},
  receive
    Result -> Result
  end,
  io:format("~p~n", [Result]);
removeValue(Name, Time, Type) ->
  server ! {msg, self(), rmV, Name, Time, Type},
  receive
    Result -> Result
  end,
  io:format("~p~n", [Result]).



getOneValue({X,Y}, Time, Type) ->
  server ! {msg, self(), get1V, {X,Y}, Time, Type},
  receive
    Result -> Result
  end,
  io:format("~p~n", [Result]);
getOneValue(Name, Time, Type) ->
  server ! {msg, self(), get1V, Name, Time, Type},
  receive
    Result -> Result
  end,
  io:format("~p~n", [Result]).



getStationMean({X,Y}, Type) ->
  server ! {msg, self(), getSMean, {X,Y}, Type},
  receive
    Result -> Result
  end,
  io:format("~p~n", [Result]);
getStationMean(Name, Type) ->
  server ! {msg, self(), getSMean, Name, Type},
  receive
    Result -> Result
  end,
  io:format("~p~n", [Result]).



getDailyMean(Time, Type) ->
  server ! {msg, self(), getDMean, Time, Type},
  receive
    Result -> Result
  end,
  io:format("~p~n", [Result]).



getMinimumPollutionStation(Type) ->
  server ! {msg, self(), getMinS, Type},
  receive
    Result -> Result
  end,
  io:format("~p~n", [Result]).






