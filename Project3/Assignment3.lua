--Include Luke's HFA Driver
darwin = require('hfaDriver')

--[[
-- Behaviors
-- localizeWander --Used to find out where we are
-- faceXY         --Used to face to a position
-- moveToXY       --Used to go to a position
-- Stop           --Stop the robot
--]]

func = function (hfa) end

localize = {}
faceXY = {}
moveToXY = {}
stopRobot = {}

xPrime = 0;
yPrime = 0;
aPrime = nil;


behaviors = {localize, faceXY, moveToXY, stopRobot}
states = {"start", "go", "stop"}

for k,v in ipairs(behaviors) do
  for i,t in ipairs(states) do
    if v[t] == nil
    then
      v[t] = func;
    end
  end
end


-- Look for the goal
localize["start"] = function (hfa)
  darwin.lookGoal();
end

-- Find out where we are and where the target is relative to us
faceXY["start"] = function(hfa)
  vector = wcm.get_pose();
  x = vector.x;
  y = vector.y;
  a = vector.a;

  deltaX = xPrime - x;
  deltaY = yPrime - y;

  aPrime = math.atan2(deltaY, deltaX);

  aPrime = aPrime * (180/math.pi);

  -- Rotate until we are facing where we want to be facing
  darwin.setVelocity(0,0, 0.1);
end


-- Move to the thing we are facing
moveToXY["start"] = function(hfa) 
  -- Want to stop all prior movement
  darwin.stop();
  darwin.setVelocity(0.1,0,0);
end

-- Stop the robot from doing shenanigans
stopRobot["start"] = function(hfa)
  darwin.setVelocity(0,0,0);
  darwin.stop();
end

-- Make the behaviors
localize_b  = makeBehavior("localize", localize["start"], localize["go"], localize["stop"]);
faceXY_b    = makeBehavior("faceXY", faceXY["start"], faceXY["go"], faceXY["stop"]);
moveToXY_b  = makeBehavior("moveToXY", moveToXY["start"], moveToXY["go"], moveToXY["stop"]);
stopRobot_b = makeBehavior("stopRobot", stopRobot["start"], stopRobot["go"], stopRobot["stop"]);

-- Make the HFA
counter = 0;
machine = makeHFA("machine", makeTransition({
  [start] = localize_b,
  [localize_b] = function() -- if we're in localize
    return faceXY_b -- assume we've updated our position
  end,

  [faceXY_b] = function() -- if we're in faceXY
    vector = wcm.get_pose();
    x = vector.x;
    y = vector.y;
    a = vector.a;

    deltaX = xPrime - x;
    deltaY = yPrime - y;

    aPrime = math.atan2(deltaY, deltaX);

    aPrime = aPrime * (180/math.pi);

    if wcm.get_pose().a == aPrime then
      return moveToXY_b
    else
      return faceXY_b
    end
  end,

  [moveToXY_b] = function()
    vector = wcm.get_pose();
    if vector.x == x and vector.y = y then
      return stopRobot_b;
    elseif counter == 100 then
      counter = 0;
      return localize_b;
    else
      counter += 1;
      return moveToXY_b;
    end
  end
}))
