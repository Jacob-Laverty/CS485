--Include Luke's HFA Driver
darwin = require('hfaDriver')


--task 2, find ball and stop near it
--[[
-- BEHAVIORS
-- localize		--wander if the ball is not seen
-- faceXY  		--X, Y are wcm.get_ball_x/y()
-- moveToXY		--used to move to a distance near the ball
-- Stop			--Stop the robot
--]]

func = function (hfa) end

init_j = {}
localize = {}
faceXY = {}
moveToXY = {}
stopRobot = {}

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

--Init start. Literally just move forward for a bit, get the darwin going
init_j["start"] = function(hfa) 
	print("Init start");
	--darwin.lookGoal();
	darwin.setVelocity(0.03,0, 0);
end

-- Look for the ball
localize["start"] = function (hfa)
  print("Looking for ball");
  darwin.scan();
end

-- Find the ball and track it
faceXY["start"] = function (hfa)
  print("facing ball");
  v = wcm.get_pose();
  x = v.x;
  y = v.y;
  a = v.a;
  
  --turn to face the ball
  deltaX = wcm.get_ball_x() - x;
  deltaY = wcm.get_ball_y() - y;
  
  angle = math.atan2(deltaX, deltaY);
  
  --rotate to face ball direction
  print("I am rotating to this angle: " .. aPrime .. "\n");
  darwin.setVelocity(0,0,0.1);
end

-- Move to near the ball
moveToXY["start"] = function (hfa)
  print("moving to ball");
  --stop prior movement
  darwin.stop();
  darwin.setVelocity(0.1, 0, 0);
end

-- Stop darwin
stopRobot["start"] = function (hfa)
  print("stopping");
  darwin.stop();
end

-- Make the behaviors
init_jb 	= makeBehavior("init_j", init_j["start"], init_j["go"], init_j["stop"]);
localize_b 	= makeBehavior("localize", localize["start"], localize["go"], localize["stop"]);
faceXY_b 	= makeBehavior("faceXY", faceXY["start"], faceXY["go"], faceXY["stop"]);
moveToXY_b  	= makeBehavior("moveToXY", moveToXY["start"], moveToXY["go"], moveToXY["stop"]);
stopRobot_b 	= makeBehavior("stopRobot", stopRobot["start"], stopRobot["go"], stopRobot["stop"]);

-- Make the HFA 
machine = makeHFA("machine", makeTransition({
  [start] = init_jb;
	[init_jb] = function()
		print("init_jb");
		if vcm.get_ball_detect() == 0 then   --cannot see ball, keep searching
			return localize_b;
		else   --can see ball, face it
			darwin.track();
			return faceXY_b;
		end
	end,
	[faceXY_b] = function()  --if we're in faceXY
		print("faceXY, facing ball");
		x=v.x
		y=v.y;
		a=v.a;
		
		deltaX = vcm.get_ball_x() - x;
		deltaY = vcm.get_ball_y() - y;
		
		angle = math.atan2(deltaY, deltaX);
		    print("Desired angle: " .. angle .. "\n");
		    
		if math.abs(wcm.get_pose().a - angle ) < 0.5 then
		  return moveToXY_b;
		else
		  return faceXY_b;
		end
	end,
	      
	[moveToXY_b] = function()
		v=wcm.get_pose();
		-- get close enough to kick
		if v.x == x and v.y == y then
		  return stopRobot_b;
		else
		  return moveToXY_b;
		end
	end
	}))
	
--start main  
darwin.executeMachine(machine);
  
