# Vanguard Auto-Turn Off (VATO)
This is a program that aims to tie the lifetime of Vanguard to the Riot Client. The moment that you close Riot client it will kill vanguard and stop it from starting on boot. That way you only use Vanguard when you are playing. Currently it's written to only work for League of Legends.  
  
**Important: There will be an ok prompt saying vanguard is disabled if it is. If you do not see this it means Riot Client and Vanguard are still running in the background.**

# How to Use
This program is designed so you can just replace the League shortcut with it and it will do everything automatically. To do so:  
  
1. Make a shortcut to VATO after downloading it
2. Go to you League of Legends or Riot Client shortcut and open its properties. In the shortcut tab copy the RiotClientServices directory (it should look something like `"E:\Riot Games\Riot Client\RiotClientServices.exe"`, making sure it has the quotes if there's spaces). In the VATO shortcut put it in target as an argument. So it should look like `E:\Path\To\VATO.exe "E:\Riot Games\Riot Client\RiotClientServices.exe"`
3. Importantly go to the compatability tab in the shortcuts properties and make sure you check `Run this program as an administrator`. VATO requires admin permissions since it needs to disble the vgc and vgk services to turn off Vanguard.
4. Place this shortcut wherever you wish! Start menu, Desktop, whatever is convinent. Then just use the shortcut whenever you want to play League and it will handle everything.
