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

init_j = {}
localize = {}
faceXY = {}
moveToXY = {}
stopRobot = {}

xPrime = 0;
yPrime = 0;
aPrime = 0;


behaviors = {init_j, localize, faceXY, moveToXY, stopRobot}
states = {"start", "go", "stop"}

for k,v in ipairs(behaviors) do
  for i,t in ipairs(states) do
    if v[t] == nil
    then
      v[t] = function(hfa) end;
    end
  end
end


--Init start. Literally just move forward for a bit
init_j["start"] = function(hfa) 
	print("Init start");
	darwin.lookGoal();
	darwin.setVelocity(0.03,0, 0);
end

-- Look for the goal
localize["start"] = function (hfa)
  print("Localize start");
  darwin.lookGoal();
end

-- Find out where we are and where the target is relative to us
faceXY["start"] = function(hfa)
  print("face start");
  v = wcm.get_pose();
  x = v.x;
  y = v.y;
  a = v.a;

  deltaX = xPrime - x;
  deltaY = yPrime - y;

  aPrime = math.atan2(deltaY, deltaX);

  --aPrime = aPrime * (180/math.pi);

  -- Rotate until we are facing where we want to be facing
  print("I need to go to this angle: ");
  print(aPrime);
  darwin.setVelocity(0,0,0.1);
end


-- Move to the thing we are facing
moveToXY["start"] = function(hfa) 
  print("move start");
  -- Want to stop all prior movement
  darwin.stop();
  darwin.setVelocity(0.1,0,0);
end

-- Stop the robot from doing shenanigans
stopRobot["start"] = function(hfa)
  print("stop start");
  darwin.setVelocity(0,0,0);
  darwin.stop();
end

-- Make the behaviors
localize_b  = makeBehavior("localize", localize["start"], localize["go"], localize["stop"]);
faceXY_b    = makeBehavior("faceXY", faceXY["start"], faceXY["go"], faceXY["stop"]);
moveToXY_b  = makeBehavior("moveToXY", moveToXY["start"], moveToXY["go"], moveToXY["stop"]);
stopRobot_b = makeBehavior("stopRobot", stopRobot["start"], stopRobot["go"], stopRobot["stop"]);
init_jb 		= makeBehavior("init_j", init_j["start"], init_j["go"], init_j["stop"]);

-- Make the HFA
counter = 0;
oldX = 0;
oldY = 0;
oldA = 0;

startup = os.time();
machine = makeHFA("machine", makeTransition({
  [start] = init_jb,
	[init_jb] = function() 
		print("init_jb")
		if os.difftime(os.time(), startup) > 15 then
			return localize_b
		else
			return init_jb
		end	
	end,

  [localize_b] = function() -- if we're in localize
		print("start");
		v = wcm.get_pose();

		xDiff =  math.abs((oldX - v.x) / ((oldX + v.x)/2));
		yDiff =  math.abs((oldY - v.y) / ((oldY + v.y)/2));

		avgDiff = (xDiff + yDiff)/2;

		print("Average difference between localize: ");
		print(avgDiff);

		-- If I consistently think I am in the same place
		if(avgDiff < 5)  then
			return faceXY_b	
		else
			return localize_b;
		end
  end,

  [faceXY_b] = function() -- if we're in faceXY
    v = wcm.get_pose();
    x = v.x;
    y = v.y;
    a = v.a;

    deltaX = xPrime - x;
    deltaY = yPrime - y;

    aPrime = math.atan2(deltaY, deltaX);

		print("Current angle: ");
		print(aPrime);

    --aPrime = aPrime * (180/math.pi);

    if math.abs(wcm.get_pose().a - aPrime) < 0.5 then
      return moveToXY_b
    else
      return faceXY_b
    end
  end,

  [moveToXY_b] = function()
    v = wcm.get_pose();
    if v.x == x and v.y == y then
      return stopRobot_b;
    elseif counter == 100 then
      counter = 0;
      return localize_b;
    else
      counter = counter + 1;
      return moveToXY_b;
    end
  end
}))

darwin.executeMachine(machine);
