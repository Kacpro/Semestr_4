
-module(pollution).

-export([createMonitor/0, addStation/3, addValue/5, removeValue/4, getOneValue/4, getStationMean/3, getDailyMean/3, getMinimumPollutionStation/2]).

-record(measurment, {time, type, value}).

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
    "" -> "Nope";
    Key ->
      {_, List} = maps:get(Key, Monitor),
      case lists:any(fun (M) -> (M#measurment.time == Time) and (M#measurment.type == Type) end, List) of
        true -> "Nope";
        _ -> Monitor#{Key => {{X,Y}, [ #measurment{time = Time, type = Type, value = Value} | List]}}
      end
  end;

addValue(Name, Time, Type, Val, Monitor) ->
  Value = float(Val),
  case maps:get(Name, Monitor, error) of
    error -> "Nope";
    {Location, List} ->
      case lists:any(fun (M) -> (M#measurment.time == Time) and (M#measurment.type == Type) end, List) of
        true -> "Nope";
        _ -> Monitor#{Name => {Location, [ #measurment{time = Time, type = Type, value = Value} | List]}}
      end
  end.



removeValue({X,Y}, Time, Type, Monitor) ->
  K = getName({X,Y}, Monitor),
  case K of
    "" -> "Nope";
    Key ->
      {_, List} = maps:get(Key, Monitor),
      Monitor#{Key => {{X,Y}, lists:filter(fun (M) -> (M#measurment.type /= Type) or (M#measurment.time /= Time) end, List)}}
  end;

removeValue(Name, Time, Type, Monitor) ->
  case maps:get(Name, Monitor, error) of
    error -> "Nope";
    {Location, List} ->
      Monitor#{Name => {Location, lists:filter(fun (M) -> (M#measurment.type /= Type) or (M#measurment.time /= Time) end, List)}}
  end.



getOneValue({X,Y}, Time, Type, Monitor) ->
  Key = getName({X,Y}, Monitor),
  case maps:get(Key, Monitor, error) of
    error -> "Nope1";
    {_,[]} -> "Nope2";
    {_, List} ->
      [#measurment{value = Value}] = lists:filter(fun (M) -> (M#measurment.type == Type) and (M#measurment.time == Time) end, List),
      Value
end;

getOneValue(Name, Time, Type, Monitor) ->
  case maps:get(Name, Monitor, error) of
    error -> "Nope";
    {_,[]} -> "Nope";
    {_, List} ->
      [#measurment{value = Value}] = lists:filter(fun (M) -> (M#measurment.type == Type) and (M#measurment.time == Time) end, List),
      Value
  end.



getStationMean({X,Y}, Type, Monitor) ->
  Key = getName({X,Y}, Monitor),
  case maps:get(Key, Monitor, error) of
    error -> "Nope1";
    {_, []} -> 0;
    {_, List} ->
      List1 = lists:filter(fun (M) -> M#measurment.type == Type end, List),
      lists:foldr(fun (M, Acc) -> M#measurment.value+Acc end, 0, List1)/(erlang:length(List1))
  end;

getStationMean({X,Y}, Type, Monitor) ->
  Key = getName({X,Y}, Monitor),
  case maps:get(Key, Monitor, error) of
    error -> "Nope1";
    {_, []} -> 0;
    {_, List} ->
      List1 = lists:filter(fun (M) -> M#measurment.type == Type end, List),
      lists:foldr(fun (M, Acc) -> M#measurment.value+Acc end, 0, List1)/(erlang:length(List1))
  end.



getDailyMean(Time, Type, Monitor) ->
  List1 = lists:map(fun ({_,{_, L}}) -> L end, maps:to_list(Monitor)),
  case lists:filter(fun (M) -> (M#measurment.type == Type) and (
    case M#measurment.time of
      {Time, _} -> true;
      _ -> false
    end )
                    end, lists:flatten(List1)) of
    [] -> 0;
    List -> lists:foldr(fun (M, Acc) -> M#measurment.value+Acc end, 0, List)/(erlang:length(List))
  end.



head([H|_]) -> H.



getMinimumPollutionStation(Type, Monitor) ->
  List = lists:map(fun ({N,{_, L}}) -> {N,L} end, maps:to_list(Monitor)),
  List1 = lists:map(fun ({N,L}) -> {N, (head(lists:filter(fun (M) -> M#measurment.type == Type end, L)))#measurment.value} end,List),
  {Name, _} = lists:foldr(fun ({N,V}, {N1, V1}) -> case V<V1 of
                                                    true -> {N,V};
                                                    _ -> {N1, V1}
                                                  end end, head(List1) ,List1),
  Name.



























