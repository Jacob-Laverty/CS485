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
	print("looking for ball in init_j");
	darwin.setVelocity(0,0, 0.1);
	darwin.scan();
end

-- Find the ball and track it
faceXY["start"] = function (hfa)
  print("attempting to face ball and tracking");
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
  darwin.setVelocity(0,0,0.1);  --rotate at 0.1 meters per second
end

-- Move to near the ball
moveToXY["start"] = function (hfa)
  v=wcm.get_pose();
  print("moving to ball");
  --stop prior movement
  darwin.stop();
  darwin.setVelocity(0.1, 0, 0);  --move forward at 0.1 meters per second
end

-- Stop darwin
stopRobot["start"] = function (hfa)
  print("stopping");
  darwin.stop();
end

-- Make the behaviors
init_jb 	= makeBehavior("init_j", init_j["start"], init_j["go"], init_j["stop"]);
faceXY_b 	= makeBehavior("faceXY", faceXY["start"], faceXY["go"], faceXY["stop"]);
moveToXY_b  	= makeBehavior("moveToXY", moveToXY["start"], moveToXY["go"], moveToXY["stop"]);
stopRobot_b 	= makeBehavior("stopRobot", stopRobot["start"], stopRobot["go"], stopRobot["stop"]);

-- Make the HFA 
machine = makeHFA("machine", makeTransition({
  [start] = init_jb;
	[init_jb] = function()
		print("init_jb");
		if vcm.get_ball_detect() == 0 then   --cannot see ball, keep searching
			return init_jb;
		else   --can see ball, face it
			return faceXY_b;
		end
	end,
	[faceXY_b] = function()  --if we're in faceXY
			    
		if math.abs(wcm.get_pose().a - angle ) < 0.5 then --turned at an angle close to the ball
		  return moveToXY_b;  --move forward to it
		else if vcm.get_ball_detect() == 0 then --lost the ball, find it again
		  return init_jb;
		else  --found ball, face it
		  return faceXY_b;
		end
	end,
	      
	[moveToXY_b] = function()
		-- get close enough to kick
		if vcm.get_ball_detect() == 0 then --lost ball again, find it
		  return init_jb;
		else if v.x == x and v.y == y then
		  return stopRobot_b;
		else
		  return moveToXY_b;
		end
	end
	
	[stopRobot_b] = function()
		if vcm.get_ball_detect()==0 then --lost the ball, find it
		  return init_jb;
		else if math.abs(wcm.get_pose().a - angle) > 0.5 then  --somehow it is at a different angle now, face it again
		  return faceXY_b;
		else if v.x ~=x and v.y ~= y then --ball must have moved, walk toward it
		  return moveToXY_b;
		else
		  return stopRobot_b;
	}))
	
--start main  
darwin.executeMachine(machine);
  
