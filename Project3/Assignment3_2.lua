--Include Luke's HFA Driver
darwin = require('hfaDriver')


--task 2, find ball and stop near it
--[[
-- BEHAVIORS
-- localize		--wander if the ball is not seen
-- faceBall  		--X, Y are wcm.get_ball_x/y()
-- moveToBall		--used to move to a distance near the ball
-- Stop			--Stop the robot
--]]

func = function (hfa) end

initRobot = {}
localize = {}
findBall = {}
faceBall = {}
moveToBall = {}
stopRobot = {}

behaviors = {initRobot, findBall, localize, faceBall, moveToBall, stopRobot}
states = {"start", "go", "stop"}

for k,v in ipairs(behaviors) do
  for i,t in ipairs(states) do
    if v[t] == nil
    then
      v[t] = function(hfa) end;
    end
  end
end

initRobot["start"] = function(hfa)
	darwin.lookGoal();
	darwin.setVelocity(0.01,0,0); --Move forward

end

--localize

localize["start"] = function(hfa)
	darwin.lookGoal();
end

--Init start. Literally just move forward for a bit, get the darwin going
findBall["start"] = function(hfa) 
	darwin.scan(); --look for ball
	--if you can't find it start to rotate
	if darwin.isBallLost() == 1 then
		darwin.stop();
		darwin.setVelocity(0,0, 0.2);
		end
end

-- Find the ball and track it
faceBall["start"] = function (hfa)
  v = wcm.get_pose();
  x = v.x;
  y = v.y;
  a = v.a;
  
  --turn to face the ball
  deltaX = wcm.get_ball_x() - x;
  deltaY = wcm.get_ball_y() - y;
  darwin.stop();  
  angle = math.atan2(deltaX, deltaY);
 		if (angle > a and (angle - math.pi) < a) or (a > angle and (a -  math.pi) <angle)  then
	 		darwin.setVelocity(0,0, 0.1);
 		else
			darwin.setVelocity(0,0, -0.1);
	 end 
  --rotate to face ball direction
end

-- Move to near the ball
moveToBall["start"] = function (hfa)
  print("ball x location:  " .. wcm.get_ball_x() .. " and ball y location: " .. wcm.get_ball_y());
	darwin.stop();
  darwin.setVelocity(0.05,0, 0);  --move forward at 0.03 meters per second
end

-- Stop darwin
stopRobot["start"] = function (hfa)
  darwin.stop();
end

-- Make the behaviors
initRobot_b 	= makeBehavior("initRobot", initRobot["start"], initRobot["go"], initRobot["stop"]);
localize_b = makeBehavior("localize", localize["start"], localize["go"], localize["stop"]);
findBall_b 	= makeBehavior("findBall", findBall["start"], findBall["go"], findBall["stop"]);
faceBall_b 	= makeBehavior("faceBall", faceBall["start"], faceBall["go"], faceBall["stop"]);
moveToBall_b  	= makeBehavior("moveToBall", moveToBall["start"], moveToBall["go"], moveToBall["stop"]);
stopRobot_b 	= makeBehavior("stopRobot", stopRobot["start"], stopRobot["go"], stopRobot["stop"]);


oldX = 0;
oldY = 0;
robotStartTime = os.time();
robotTransitionDelay = 0;

-- Make the HFA 
machine = makeHFA("machine", makeTransition({
  [start] = initRobot_b;

	[initRobot_b] = function()
		if(os.difftime(os.time(), robotStartTime) > 20 ) then 
			return localize_b;
		else
			return initRobot_b;
		end
	end,

	[localize_b] = function()
		v=wcm.get_pose();

		xDiff = math.abs((oldX - v.x) / ((oldX + v.x)/2));
		yDiff = math.abs((oldY - v.y) / ((oldY + v.y)/2));

		avgDiff = (xDiff + yDiff) /2;

		if(avgDiff < 5) then
			return findBall_b;
		else
			return localize_b;
		end
	end,

	[findBall_b] = function()
		if darwin.isBallLost() == 1 then   --cannot see ball, keep searching
			return findBall_b;
		else   --can see ball, face it
			darwin.stop();
			if vcm.get_ball_detect == 1 then
				darwin.track();
			end
			robotTransitionDelay = os.time();
			if (os.difftime(os.time(), robotTansitionDelay) > 3) then
				print("facing ball");
				return faceBall_b;
			end
		end
	end,

	[faceBall_b] = function()  --if we're in faceXY
		-- Assume we rotoated to the correct angle
		v=wcm.get_pose();
		x=v.x;
		y=v.y;
		a=v.a;
		print(math.abs(a-angle) .. " is the angle difference.");
		if math.abs(a - angle ) < 0.05 then --turned at an angle close to the ball
			print("current x: " .. v.x .. " and current y: " .. v.y);
			return moveToBall_b;
		else  --found ball, face it
		  return faceBall_b;
		end
	end,
	      
	[moveToBall_b] = function()
	  v=wcm.get_pose();
		x=v.x;
		y=v.y;
		a=v.a;
		xPosSquared = math.pow((wcm.get_ball_x() - x), 2);
		yPosSquared = math.pow((wcm.get_ball_y() - y), 2);
		distance = math.sqrt(xPosSquared + yPosSquared);
		-- get close enough to kick
		if distance < 0.25 then
			print("difference in x: " .. math.abs(v.x-x) .. " difference in y" .. math.abs(v.y-y));
		  return stopRobot_b;
		else --not there yet keep moving forward
		  return moveToBall_b;
		end
	end,
	
	[stopRobot_b] = function()
			darwin.stop();
			return stopRobot_b;
	end
	}),false);
	
--start main  
darwin.executeMachine(machine);
