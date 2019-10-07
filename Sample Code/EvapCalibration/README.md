---Process for Calibrating Load Cell---
1. Create spreadsheet to record measured digital value vs added weight
2. Run code and add weights at even intervals(weight maxes out at 850 grams
3. Record each measured digital value vs added weight into spreadsheet
4. Use built in linear regression tools to determine a slope and intercept
5. Input slope and intercept into code and change CALIBRATE to 0 and CHECK to 1 to
   run the code in the check mode
6. Verify the calculated linear equation is correct by re-adding weights and checking
   that the code calculates an accurate value
