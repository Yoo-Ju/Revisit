### 연구 목적: Indoor moving patterns을 이용하여 revisit intention을 예측.


 
#### 어떤 유저의 Revisit intention(Revisit count)을 예측하는 supervised learning(regression) 모델에 이용될 간단한 Feature들:
1. User가 찍힌 로그 총 개수: num_logs
2. 한 User가 와이파이에 잡힌 총 시간: total_dwell_time
3. dwell_time > 10인 indoor area 개수: num_sp_100
4. indoor 로그 중 dwell_time > 10인 확률: prob_dwell_10
5. 전체 로그 중 deny=True(직원)인 확률: prob_deny
6. dwell_time > 100인 indoor area에서 보낸 total time: time_sp_100
7. dwell_time > 100인 indoor area들의 variance: std_sp_100



#### 여기서의 초점
1. 한 유저에 대한 log가 여러 개인데, 일단 aggregate하여 유저당 하나의 row로 나타낼 수 있도록 함)
2. 가지고 있는 데이터셋의 revisit_count는 같은 날 두 번 방문하면 올라가지 않는다 - 유저별 max(revisit_count) > 0 이면 yes, 아니면 no로 레이블링
3. 정확한 트레이닝은, 특정 시점 이전의 데이터를 이용하여 특정 시점 이후에 고객이 방문했는지 안 했는지 조사해야 하지만, 귀찮으므로 일단은 지금까지 전체 데이터를 가지고 revisit count를 예측하는 모델로 구성



#### 데이터 
1. 781, 786번 매장 데이터 (10%의 유저를 테스트로 이용)



---


시간을 정보를 모조리 뭉뚱그린 하나의 feature로 아래 그림과 같은 sequence를 나타내긴 아까운데...
![예제](../notebook/20160830_223133_HDR.jpg)

### 방법이 없을까?
Temporally annotated sequence를 feature로 사용하는 방법?

1. [A Brief Survey on Sequence Classification](../documents/Sequence_Classification.pdf)