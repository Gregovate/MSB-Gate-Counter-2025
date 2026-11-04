# Source index and assessment method
All references are immutable GitHub source commits. Line anchors identify review evidence, not runtime measurements. The source snapshots were retrieved through the authenticated GitHub connection; private configuration was not retrieved.
## C — MSB-Car-Counter-2024
Ref: `5a01aec573f2b08c6cc3f36995af3aedd42e56c9`. [Complete source](https://github.com/Gregovate/MSB-Car-Counter-2024/blob/5a01aec573f2b08c6cc3f36995af3aedd42e56c9/src/main.cpp).
| Function | Source location |
|---|---|
| onOTAStart | [L534–L538](https://github.com/Gregovate/MSB-Car-Counter-2024/blob/5a01aec573f2b08c6cc3f36995af3aedd42e56c9/src/main.cpp#L534-L538) |
| onOTAProgress | [L540–L546](https://github.com/Gregovate/MSB-Car-Counter-2024/blob/5a01aec573f2b08c6cc3f36995af3aedd42e56c9/src/main.cpp#L540-L546) |
| onOTAEnd | [L548–L556](https://github.com/Gregovate/MSB-Car-Counter-2024/blob/5a01aec573f2b08c6cc3f36995af3aedd42e56c9/src/main.cpp#L548-L556) |
| getRtcTimestamp | [L566–L578](https://github.com/Gregovate/MSB-Car-Counter-2024/blob/5a01aec573f2b08c6cc3f36995af3aedd42e56c9/src/main.cpp#L566-L578) |
| checkSdHealth | [L673–L700](https://github.com/Gregovate/MSB-Car-Counter-2024/blob/5a01aec573f2b08c6cc3f36995af3aedd42e56c9/src/main.cpp#L673-L700) |
| publishSdDiag_ | [L714–L740](https://github.com/Gregovate/MSB-Car-Counter-2024/blob/5a01aec573f2b08c6cc3f36995af3aedd42e56c9/src/main.cpp#L714-L740) |
| SetLocalTime | [L766–L804](https://github.com/Gregovate/MSB-Car-Counter-2024/blob/5a01aec573f2b08c6cc3f36995af3aedd42e56c9/src/main.cpp#L766-L804) |
| setup_wifi | [L809–L879](https://github.com/Gregovate/MSB-Car-Counter-2024/blob/5a01aec573f2b08c6cc3f36995af3aedd42e56c9/src/main.cpp#L809-L879) |
| preferPrimaryIfAvailable | [L884–L899](https://github.com/Gregovate/MSB-Car-Counter-2024/blob/5a01aec573f2b08c6cc3f36995af3aedd42e56c9/src/main.cpp#L884-L899) |
| buildPath | [L906–L911](https://github.com/Gregovate/MSB-Car-Counter-2024/blob/5a01aec573f2b08c6cc3f36995af3aedd42e56c9/src/main.cpp#L906-L911) |
| ensureSeasonFolderExists | [L914–L929](https://github.com/Gregovate/MSB-Car-Counter-2024/blob/5a01aec573f2b08c6cc3f36995af3aedd42e56c9/src/main.cpp#L914-L929) |
| listSDFiles | [L932–L984](https://github.com/Gregovate/MSB-Car-Counter-2024/blob/5a01aec573f2b08c6cc3f36995af3aedd42e56c9/src/main.cpp#L932-L984) |
| downloadSDFile | [L989–L1019](https://github.com/Gregovate/MSB-Car-Counter-2024/blob/5a01aec573f2b08c6cc3f36995af3aedd42e56c9/src/main.cpp#L989-L1019) |
| changeDirectory | [L1074–L1108](https://github.com/Gregovate/MSB-Car-Counter-2024/blob/5a01aec573f2b08c6cc3f36995af3aedd42e56c9/src/main.cpp#L1074-L1108) |
| deleteSDFile | [L1110–L1138](https://github.com/Gregovate/MSB-Car-Counter-2024/blob/5a01aec573f2b08c6cc3f36995af3aedd42e56c9/src/main.cpp#L1110-L1138) |
| renameSDFile | [L1140–L1183](https://github.com/Gregovate/MSB-Car-Counter-2024/blob/5a01aec573f2b08c6cc3f36995af3aedd42e56c9/src/main.cpp#L1140-L1183) |
| setupServer | [L1189–L1404](https://github.com/Gregovate/MSB-Car-Counter-2024/blob/5a01aec573f2b08c6cc3f36995af3aedd42e56c9/src/main.cpp#L1189-L1404) |
| encodeQueuedMessage | [L1419–L1421](https://github.com/Gregovate/MSB-Car-Counter-2024/blob/5a01aec573f2b08c6cc3f36995af3aedd42e56c9/src/main.cpp#L1419-L1421) |
| publishMQTT | [L1440–L1442](https://github.com/Gregovate/MSB-Car-Counter-2024/blob/5a01aec573f2b08c6cc3f36995af3aedd42e56c9/src/main.cpp#L1440-L1442) |
| publishQueuedMessages | [L1445–L1473](https://github.com/Gregovate/MSB-Car-Counter-2024/blob/5a01aec573f2b08c6cc3f36995af3aedd42e56c9/src/main.cpp#L1445-L1473) |
| publishDebugLog | [L1475–L1477](https://github.com/Gregovate/MSB-Car-Counter-2024/blob/5a01aec573f2b08c6cc3f36995af3aedd42e56c9/src/main.cpp#L1475-L1477) |
| publishDebugEvent | [L1483–L1502](https://github.com/Gregovate/MSB-Car-Counter-2024/blob/5a01aec573f2b08c6cc3f36995af3aedd42e56c9/src/main.cpp#L1483-L1502) |
| KeepMqttAlive | [L1505–L1587](https://github.com/Gregovate/MSB-Car-Counter-2024/blob/5a01aec573f2b08c6cc3f36995af3aedd42e56c9/src/main.cpp#L1505-L1587) |
| computeDaysRunning | [L1594–L1623](https://github.com/Gregovate/MSB-Car-Counter-2024/blob/5a01aec573f2b08c6cc3f36995af3aedd42e56c9/src/main.cpp#L1594-L1623) |
| determineSeasonYear | [L1628–L1632](https://github.com/Gregovate/MSB-Car-Counter-2024/blob/5a01aec573f2b08c6cc3f36995af3aedd42e56c9/src/main.cpp#L1628-L1632) |
| publishTimingConfig | [L1638–L1641](https://github.com/Gregovate/MSB-Car-Counter-2024/blob/5a01aec573f2b08c6cc3f36995af3aedd42e56c9/src/main.cpp#L1638-L1641) |
| MQTTreconnect | [L1650–L1755](https://github.com/Gregovate/MSB-Car-Counter-2024/blob/5a01aec573f2b08c6cc3f36995af3aedd42e56c9/src/main.cpp#L1650-L1755) |
| checkWiFiConnection | [L1758–L1781](https://github.com/Gregovate/MSB-Car-Counter-2024/blob/5a01aec573f2b08c6cc3f36995af3aedd42e56c9/src/main.cpp#L1758-L1781) |
| getDailyTotal | [L1785–L1822](https://github.com/Gregovate/MSB-Car-Counter-2024/blob/5a01aec573f2b08c6cc3f36995af3aedd42e56c9/src/main.cpp#L1785-L1822) |
| getShowTotal | [L1825–L1862](https://github.com/Gregovate/MSB-Car-Counter-2024/blob/5a01aec573f2b08c6cc3f36995af3aedd42e56c9/src/main.cpp#L1825-L1862) |
| getDayOfMonth | [L1866–L1903](https://github.com/Gregovate/MSB-Car-Counter-2024/blob/5a01aec573f2b08c6cc3f36995af3aedd42e56c9/src/main.cpp#L1866-L1903) |
| getDaysRunning | [L1908–L1947](https://github.com/Gregovate/MSB-Car-Counter-2024/blob/5a01aec573f2b08c6cc3f36995af3aedd42e56c9/src/main.cpp#L1908-L1947) |
| getHourlyData | [L1953–L2034](https://github.com/Gregovate/MSB-Car-Counter-2024/blob/5a01aec573f2b08c6cc3f36995af3aedd42e56c9/src/main.cpp#L1953-L2034) |
| saveDailyTotal | [L2040–L2069](https://github.com/Gregovate/MSB-Car-Counter-2024/blob/5a01aec573f2b08c6cc3f36995af3aedd42e56c9/src/main.cpp#L2040-L2069) |
| saveShowTotal | [L2074–L2126](https://github.com/Gregovate/MSB-Car-Counter-2024/blob/5a01aec573f2b08c6cc3f36995af3aedd42e56c9/src/main.cpp#L2074-L2126) |
| saveDayOfMonth | [L2131–L2158](https://github.com/Gregovate/MSB-Car-Counter-2024/blob/5a01aec573f2b08c6cc3f36995af3aedd42e56c9/src/main.cpp#L2131-L2158) |
| saveDaysRunning | [L2164–L2191](https://github.com/Gregovate/MSB-Car-Counter-2024/blob/5a01aec573f2b08c6cc3f36995af3aedd42e56c9/src/main.cpp#L2164-L2191) |
| saveHourlyCounts | [L2197–L2313](https://github.com/Gregovate/MSB-Car-Counter-2024/blob/5a01aec573f2b08c6cc3f36995af3aedd42e56c9/src/main.cpp#L2197-L2313) |
| saveDailyShowSummary | [L2317–L2451](https://github.com/Gregovate/MSB-Car-Counter-2024/blob/5a01aec573f2b08c6cc3f36995af3aedd42e56c9/src/main.cpp#L2317-L2451) |
| getSavedValuesOnReboot | [L2453–L2545](https://github.com/Gregovate/MSB-Car-Counter-2024/blob/5a01aec573f2b08c6cc3f36995af3aedd42e56c9/src/main.cpp#L2453-L2545) |
| callback | [L2550–L2692](https://github.com/Gregovate/MSB-Car-Counter-2024/blob/5a01aec573f2b08c6cc3f36995af3aedd42e56c9/src/main.cpp#L2550-L2692) |
| playPattern | [L2698–L2765](https://github.com/Gregovate/MSB-Car-Counter-2024/blob/5a01aec573f2b08c6cc3f36995af3aedd42e56c9/src/main.cpp#L2698-L2765) |
| averageHourlyTemp | [L2768–L2806](https://github.com/Gregovate/MSB-Car-Counter-2024/blob/5a01aec573f2b08c6cc3f36995af3aedd42e56c9/src/main.cpp#L2768-L2806) |
| countTheCar | [L2809–L2886](https://github.com/Gregovate/MSB-Car-Counter-2024/blob/5a01aec573f2b08c6cc3f36995af3aedd42e56c9/src/main.cpp#L2809-L2886) |
| detectCar | [L2889–L3077](https://github.com/Gregovate/MSB-Car-Counter-2024/blob/5a01aec573f2b08c6cc3f36995af3aedd42e56c9/src/main.cpp#L2889-L3077) |
| checkAndCreateFile | [L3086–L3149](https://github.com/Gregovate/MSB-Car-Counter-2024/blob/5a01aec573f2b08c6cc3f36995af3aedd42e56c9/src/main.cpp#L3086-L3149) |
| createAndInitializeHourlyFile | [L3154–L3191](https://github.com/Gregovate/MSB-Car-Counter-2024/blob/5a01aec573f2b08c6cc3f36995af3aedd42e56c9/src/main.cpp#L3154-L3191) |
| initSDCard | [L3197–L3349](https://github.com/Gregovate/MSB-Car-Counter-2024/blob/5a01aec573f2b08c6cc3f36995af3aedd42e56c9/src/main.cpp#L3197-L3349) |
| readTempandRH | [L3355–L3437](https://github.com/Gregovate/MSB-Car-Counter-2024/blob/5a01aec573f2b08c6cc3f36995af3aedd42e56c9/src/main.cpp#L3355-L3437) |
| resetHourlyCounts | [L3442–L3449](https://github.com/Gregovate/MSB-Car-Counter-2024/blob/5a01aec573f2b08c6cc3f36995af3aedd42e56c9/src/main.cpp#L3442-L3449) |
| timeTriggeredEvents | [L3452–L3556](https://github.com/Gregovate/MSB-Car-Counter-2024/blob/5a01aec573f2b08c6cc3f36995af3aedd42e56c9/src/main.cpp#L3452-L3556) |
| updateDisplay | [L3561–L3593](https://github.com/Gregovate/MSB-Car-Counter-2024/blob/5a01aec573f2b08c6cc3f36995af3aedd42e56c9/src/main.cpp#L3561-L3593) |
| setup | [L3596–L3770](https://github.com/Gregovate/MSB-Car-Counter-2024/blob/5a01aec573f2b08c6cc3f36995af3aedd42e56c9/src/main.cpp#L3596-L3770) |
| loop | [L3772–L3838](https://github.com/Gregovate/MSB-Car-Counter-2024/blob/5a01aec573f2b08c6cc3f36995af3aedd42e56c9/src/main.cpp#L3772-L3838) |
## G — MSB-Gate-Counter-2025
Ref: `4b18fe284591f5f2aa58e3af1269ece75eaae003`. [Complete source](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/4b18fe284591f5f2aa58e3af1269ece75eaae003/src/main.cpp).
| Function | Source location |
|---|---|
| onOTAStart | [L475–L479](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/4b18fe284591f5f2aa58e3af1269ece75eaae003/src/main.cpp#L475-L479) |
| onOTAProgress | [L481–L487](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/4b18fe284591f5f2aa58e3af1269ece75eaae003/src/main.cpp#L481-L487) |
| onOTAEnd | [L489–L497](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/4b18fe284591f5f2aa58e3af1269ece75eaae003/src/main.cpp#L489-L497) |
| getRtcTimestamp | [L507–L519](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/4b18fe284591f5f2aa58e3af1269ece75eaae003/src/main.cpp#L507-L519) |
| sPath | [L609–L611](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/4b18fe284591f5f2aa58e3af1269ece75eaae003/src/main.cpp#L609-L611) |
| SetLocalTime | [L635–L673](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/4b18fe284591f5f2aa58e3af1269ece75eaae003/src/main.cpp#L635-L673) |
| setup_wifi | [L676–L713](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/4b18fe284591f5f2aa58e3af1269ece75eaae003/src/main.cpp#L676-L713) |
| determineSeasonYear | [L720–L725](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/4b18fe284591f5f2aa58e3af1269ece75eaae003/src/main.cpp#L720-L725) |
| ensureSeasonFolderExists | [L727–L738](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/4b18fe284591f5f2aa58e3af1269ece75eaae003/src/main.cpp#L727-L738) |
| initSeasonalPaths | [L750–L780](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/4b18fe284591f5f2aa58e3af1269ece75eaae003/src/main.cpp#L750-L780) |
| buildPath | [L786–L791](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/4b18fe284591f5f2aa58e3af1269ece75eaae003/src/main.cpp#L786-L791) |
| listSDFiles | [L795–L843](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/4b18fe284591f5f2aa58e3af1269ece75eaae003/src/main.cpp#L795-L843) |
| downloadSDFile | [L847–L866](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/4b18fe284591f5f2aa58e3af1269ece75eaae003/src/main.cpp#L847-L866) |
| uploadSDFile | [L868–L904](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/4b18fe284591f5f2aa58e3af1269ece75eaae003/src/main.cpp#L868-L904) |
| changeDirectory | [L905–L948](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/4b18fe284591f5f2aa58e3af1269ece75eaae003/src/main.cpp#L905-L948) |
| deleteSDFile | [L950–L978](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/4b18fe284591f5f2aa58e3af1269ece75eaae003/src/main.cpp#L950-L978) |
| renameSDFile | [L980–L1019](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/4b18fe284591f5f2aa58e3af1269ece75eaae003/src/main.cpp#L980-L1019) |
| setupServer | [L1023–L1193](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/4b18fe284591f5f2aa58e3af1269ece75eaae003/src/main.cpp#L1023-L1193) |
| encodeQueuedMessage | [L1206–L1208](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/4b18fe284591f5f2aa58e3af1269ece75eaae003/src/main.cpp#L1206-L1208) |
| saveShowStartDate | [L1210–L1216](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/4b18fe284591f5f2aa58e3af1269ece75eaae003/src/main.cpp#L1210-L1216) |
| getShowStartDate | [L1218–L1237](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/4b18fe284591f5f2aa58e3af1269ece75eaae003/src/main.cpp#L1218-L1237) |
| publishMQTT | [L1257–L1259](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/4b18fe284591f5f2aa58e3af1269ece75eaae003/src/main.cpp#L1257-L1259) |
| publishQueuedMessages | [L1262–L1290](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/4b18fe284591f5f2aa58e3af1269ece75eaae003/src/main.cpp#L1262-L1290) |
| publishDebugLog | [L1292–L1294](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/4b18fe284591f5f2aa58e3af1269ece75eaae003/src/main.cpp#L1292-L1294) |
| publishDebugEvent | [L1300–L1319](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/4b18fe284591f5f2aa58e3af1269ece75eaae003/src/main.cpp#L1300-L1319) |
| KeepMqttAlive | [L1322–L1355](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/4b18fe284591f5f2aa58e3af1269ece75eaae003/src/main.cpp#L1322-L1355) |
| publishHello | [L1359–L1378](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/4b18fe284591f5f2aa58e3af1269ece75eaae003/src/main.cpp#L1359-L1378) |
| MQTTreconnect | [L1382–L1483](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/4b18fe284591f5f2aa58e3af1269ece75eaae003/src/main.cpp#L1382-L1483) |
| computeDaysRunningFromStart | [L1489–L1506](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/4b18fe284591f5f2aa58e3af1269ece75eaae003/src/main.cpp#L1489-L1506) |
| checkWiFiConnection | [L1508–L1531](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/4b18fe284591f5f2aa58e3af1269ece75eaae003/src/main.cpp#L1508-L1531) |
| getDailyTotal | [L1535–L1556](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/4b18fe284591f5f2aa58e3af1269ece75eaae003/src/main.cpp#L1535-L1556) |
| getShowTotal | [L1560–L1577](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/4b18fe284591f5f2aa58e3af1269ece75eaae003/src/main.cpp#L1560-L1577) |
| getDayOfMonth | [L1581–L1598](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/4b18fe284591f5f2aa58e3af1269ece75eaae003/src/main.cpp#L1581-L1598) |
| getDaysRunning | [L1602–L1619](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/4b18fe284591f5f2aa58e3af1269ece75eaae003/src/main.cpp#L1602-L1619) |
| getHourlyData | [L1623–L1693](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/4b18fe284591f5f2aa58e3af1269ece75eaae003/src/main.cpp#L1623-L1693) |
| saveDailyTotal | [L1697–L1710](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/4b18fe284591f5f2aa58e3af1269ece75eaae003/src/main.cpp#L1697-L1710) |
| saveShowTotal | [L1714–L1726](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/4b18fe284591f5f2aa58e3af1269ece75eaae003/src/main.cpp#L1714-L1726) |
| saveDayOfMonth | [L1730–L1742](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/4b18fe284591f5f2aa58e3af1269ece75eaae003/src/main.cpp#L1730-L1742) |
| saveDaysRunning | [L1746–L1758](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/4b18fe284591f5f2aa58e3af1269ece75eaae003/src/main.cpp#L1746-L1758) |
| saveHourlyCounts | [L1763–L1836](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/4b18fe284591f5f2aa58e3af1269ece75eaae003/src/main.cpp#L1763-L1836) |
| saveDailyShowSummary | [L1840–L1916](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/4b18fe284591f5f2aa58e3af1269ece75eaae003/src/main.cpp#L1840-L1916) |
| getSavedValuesOnReboot | [L1918–L1979](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/4b18fe284591f5f2aa58e3af1269ece75eaae003/src/main.cpp#L1918-L1979) |
| debugInPark | [L1985–L1991](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/4b18fe284591f5f2aa58e3af1269ece75eaae003/src/main.cpp#L1985-L1991) |
| topicIs | [L1995–L1997](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/4b18fe284591f5f2aa58e3af1269ece75eaae003/src/main.cpp#L1995-L1997) |
| publishStateExitInPark | [L2001–L2004](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/4b18fe284591f5f2aa58e3af1269ece75eaae003/src/main.cpp#L2001-L2004) |
| publishTimingConfig | [L2007–L2013](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/4b18fe284591f5f2aa58e3af1269ece75eaae003/src/main.cpp#L2007-L2013) |
| callback | [L2015–L2153](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/4b18fe284591f5f2aa58e3af1269ece75eaae003/src/main.cpp#L2015-L2153) |
| logSensorStates | [L2158–L2184](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/4b18fe284591f5f2aa58e3af1269ece75eaae003/src/main.cpp#L2158-L2184) |
| averageHourlyTemp | [L2186–L2227](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/4b18fe284591f5f2aa58e3af1269ece75eaae003/src/main.cpp#L2186-L2227) |
| countTheCar | [L2231–L2315](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/4b18fe284591f5f2aa58e3af1269ece75eaae003/src/main.cpp#L2231-L2315) |
| detectCar | [L2335–L2542](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/4b18fe284591f5f2aa58e3af1269ece75eaae003/src/main.cpp#L2335-L2542) |
| checkAndCreateFile | [L2549–L2571](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/4b18fe284591f5f2aa58e3af1269ece75eaae003/src/main.cpp#L2549-L2571) |
| createAndInitializeHourlyFile | [L2573–L2602](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/4b18fe284591f5f2aa58e3af1269ece75eaae003/src/main.cpp#L2573-L2602) |
| initSDCard | [L2605–L2646](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/4b18fe284591f5f2aa58e3af1269ece75eaae003/src/main.cpp#L2605-L2646) |
| readTempandRH | [L2649–L2731](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/4b18fe284591f5f2aa58e3af1269ece75eaae003/src/main.cpp#L2649-L2731) |
| resetHourlyCounts | [L2737–L2744](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/4b18fe284591f5f2aa58e3af1269ece75eaae003/src/main.cpp#L2737-L2744) |
| timeTriggeredEvents | [L2747–L2822](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/4b18fe284591f5f2aa58e3af1269ece75eaae003/src/main.cpp#L2747-L2822) |
| updateDisplay | [L2826–L2863](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/4b18fe284591f5f2aa58e3af1269ece75eaae003/src/main.cpp#L2826-L2863) |
| setup | [L2866–L3044](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/4b18fe284591f5f2aa58e3af1269ece75eaae003/src/main.cpp#L2866-L3044) |
| loop | [L3046–L3070](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/4b18fe284591f5f2aa58e3af1269ece75eaae003/src/main.cpp#L3046-L3070) |
## M — MSB-Gate-Counter-2025
Ref: `0ebacc9fa7dd29299fe0eec9b6b78a939955b670`. [Complete source](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/0ebacc9fa7dd29299fe0eec9b6b78a939955b670/src/main.cpp).
| Function | Source location |
|---|---|
| onOTAStart | [L198–L202](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/0ebacc9fa7dd29299fe0eec9b6b78a939955b670/src/main.cpp#L198-L202) |
| onOTAProgress | [L204–L210](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/0ebacc9fa7dd29299fe0eec9b6b78a939955b670/src/main.cpp#L204-L210) |
| onOTAEnd | [L212–L220](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/0ebacc9fa7dd29299fe0eec9b6b78a939955b670/src/main.cpp#L212-L220) |
| SetLocalTime | [L330–L368](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/0ebacc9fa7dd29299fe0eec9b6b78a939955b670/src/main.cpp#L330-L368) |
| setup_wifi | [L370–L407](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/0ebacc9fa7dd29299fe0eec9b6b78a939955b670/src/main.cpp#L370-L407) |
| listSDFiles | [L410–L426](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/0ebacc9fa7dd29299fe0eec9b6b78a939955b670/src/main.cpp#L410-L426) |
| downloadSDFile | [L428–L445](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/0ebacc9fa7dd29299fe0eec9b6b78a939955b670/src/main.cpp#L428-L445) |
| uploadSDFile | [L447–L481](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/0ebacc9fa7dd29299fe0eec9b6b78a939955b670/src/main.cpp#L447-L481) |
| changeDirectory | [L483–L500](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/0ebacc9fa7dd29299fe0eec9b6b78a939955b670/src/main.cpp#L483-L500) |
| deleteSDFile | [L502–L528](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/0ebacc9fa7dd29299fe0eec9b6b78a939955b670/src/main.cpp#L502-L528) |
| setupServer | [L532–L613](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/0ebacc9fa7dd29299fe0eec9b6b78a939955b670/src/main.cpp#L532-L613) |
| publishMQTT | [L620–L628](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/0ebacc9fa7dd29299fe0eec9b6b78a939955b670/src/main.cpp#L620-L628) |
| publishQueuedMessages | [L631–L643](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/0ebacc9fa7dd29299fe0eec9b6b78a939955b670/src/main.cpp#L631-L643) |
| publishDebugLog | [L645–L647](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/0ebacc9fa7dd29299fe0eec9b6b78a939955b670/src/main.cpp#L645-L647) |
| KeepMqttAlive | [L650–L655](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/0ebacc9fa7dd29299fe0eec9b6b78a939955b670/src/main.cpp#L650-L655) |
| MQTTreconnect | [L661–L734](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/0ebacc9fa7dd29299fe0eec9b6b78a939955b670/src/main.cpp#L661-L734) |
| checkWiFiConnection | [L737–L760](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/0ebacc9fa7dd29299fe0eec9b6b78a939955b670/src/main.cpp#L737-L760) |
| getDailyTotal | [L764–L780](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/0ebacc9fa7dd29299fe0eec9b6b78a939955b670/src/main.cpp#L764-L780) |
| getShowTotal | [L783–L797](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/0ebacc9fa7dd29299fe0eec9b6b78a939955b670/src/main.cpp#L783-L797) |
| getDayOfMonth | [L800–L814](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/0ebacc9fa7dd29299fe0eec9b6b78a939955b670/src/main.cpp#L800-L814) |
| getDaysRunning | [L817–L831](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/0ebacc9fa7dd29299fe0eec9b6b78a939955b670/src/main.cpp#L817-L831) |
| getHourlyData | [L834–L890](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/0ebacc9fa7dd29299fe0eec9b6b78a939955b670/src/main.cpp#L834-L890) |
| saveDailyTotal | [L894–L906](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/0ebacc9fa7dd29299fe0eec9b6b78a939955b670/src/main.cpp#L894-L906) |
| saveShowTotal | [L909–L919](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/0ebacc9fa7dd29299fe0eec9b6b78a939955b670/src/main.cpp#L909-L919) |
| saveDayOfMonth | [L922–L932](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/0ebacc9fa7dd29299fe0eec9b6b78a939955b670/src/main.cpp#L922-L932) |
| saveDaysRunning | [L935–L945](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/0ebacc9fa7dd29299fe0eec9b6b78a939955b670/src/main.cpp#L935-L945) |
| saveHourlyCounts | [L949–L1022](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/0ebacc9fa7dd29299fe0eec9b6b78a939955b670/src/main.cpp#L949-L1022) |
| saveDailyShowSummary | [L1025–L1100](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/0ebacc9fa7dd29299fe0eec9b6b78a939955b670/src/main.cpp#L1025-L1100) |
| getSavedValuesOnReboot | [L1102–L1136](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/0ebacc9fa7dd29299fe0eec9b6b78a939955b670/src/main.cpp#L1102-L1136) |
| callback | [L1140–L1207](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/0ebacc9fa7dd29299fe0eec9b6b78a939955b670/src/main.cpp#L1140-L1207) |
| logSensorStates | [L1211–L1236](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/0ebacc9fa7dd29299fe0eec9b6b78a939955b670/src/main.cpp#L1211-L1236) |
| averageHourlyTemp | [L1239–L1276](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/0ebacc9fa7dd29299fe0eec9b6b78a939955b670/src/main.cpp#L1239-L1276) |
| countTheCar | [L1279–L1344](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/0ebacc9fa7dd29299fe0eec9b6b78a939955b670/src/main.cpp#L1279-L1344) |
| detectCar | [L1351–L1471](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/0ebacc9fa7dd29299fe0eec9b6b78a939955b670/src/main.cpp#L1351-L1471) |
| checkAndCreateFile | [L1475–L1497](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/0ebacc9fa7dd29299fe0eec9b6b78a939955b670/src/main.cpp#L1475-L1497) |
| createAndInitializeHourlyFile | [L1499–L1528](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/0ebacc9fa7dd29299fe0eec9b6b78a939955b670/src/main.cpp#L1499-L1528) |
| initSDCard | [L1531–L1572](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/0ebacc9fa7dd29299fe0eec9b6b78a939955b670/src/main.cpp#L1531-L1572) |
| readTempandRH | [L1574–L1637](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/0ebacc9fa7dd29299fe0eec9b6b78a939955b670/src/main.cpp#L1574-L1637) |
| resetHourlyCounts | [L1640–L1647](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/0ebacc9fa7dd29299fe0eec9b6b78a939955b670/src/main.cpp#L1640-L1647) |
| timeTriggeredEvents | [L1650–L1727](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/0ebacc9fa7dd29299fe0eec9b6b78a939955b670/src/main.cpp#L1650-L1727) |
| updateDisplay | [L1731–L1768](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/0ebacc9fa7dd29299fe0eec9b6b78a939955b670/src/main.cpp#L1731-L1768) |
| setup | [L1771–L1896](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/0ebacc9fa7dd29299fe0eec9b6b78a939955b670/src/main.cpp#L1771-L1896) |
| loop | [L1898–L1921](https://github.com/Gregovate/MSB-Gate-Counter-2025/blob/0ebacc9fa7dd29299fe0eec9b6b78a939955b670/src/main.cpp#L1898-L1921) |

## Other reviewed evidence
- Both repositories: platformio.ini, README and project-rules portal/runbook-first rule.
- Show baselines: src/mqtt_common.h, update_version.py, data/index.html, data/showSummary.html, docs/mqtt_reset_topics.md.
- Gate example include headers were read in the preceding source reconnaissance; active main.cpp include directives confirm they are not compiled into the selected implementation.
- Branch inventories and source trees establish baseline locations and absence of tracked vendored ElegantOTAPro at the inspected refs.
- Car commit17600155505c255825ea058d5c5a4925e105be98 patch confirms the later unversioned CSV-header-only change.
- Full615-line current traffic_analytics.yaml was read from the identified Library version. No entire park HA configuration or live registry/broker was accessed.

## Validation performed
Function/call-site and literal-topic inventories were extracted from the pinned sources, then inspected against the surrounding code. Critical findings were checked against both callers and callees. Static control flow establishes possible failure paths; no new device test, firmware compilation, OTA upload or production fault injection was performed. Existing historical reliability is operator evidence, not a result generated by this assessment.
