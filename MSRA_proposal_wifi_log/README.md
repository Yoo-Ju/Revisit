## MSRA_proposal_wifi_log

### 이 문제에 대한 생각
* 현재 이용하는 데이터셋
	* 코오롱 Wi-Fi 데이터셋 
		* feature로 쓸 만한 게 너무 없음. 어디에 Wi-Fi가 찍혔느냐랑 revisit count, dwell time, revisit period밖에 없음.
		* 교수님이 생각하는 문제에 필요한 feature를 갖고 있긴 함.
* 기타 데이터셋
	* 스마트 워치 데이터셋 (2시에 이의진 교수님과 미팅하기로 함)
	* 민수형이 알려줬던 데이터셋
	* Kaggle data (이건 목적이 revisit intention이 아님 - movement pattern등을 비롯한 로그(사용 앱, 앱 카테고리, 위도 경도, 폰 기종)를 이용하여 남/녀, 나이대 category를 맞추는 게 문제)
* Feature를 잡으면 당연히 더 나아질 텐데, model를 만드는 게 아니라 feature engineering 문제라 old한 느낌.


### 일정
* 월: 엑소브레인 최종 통합
	* 솔트룩스 서버에 RDF-Unit, B-Box 설치 및 쉘 스크립트 작성, 작동 및 데이터 확인 완료
* 화: MSRA 프로포절 데이터 분석 시작: 781번 매장(커스텀멜로우 홍대점)에 대한 크롤링
	* Repo: https://github.com/Seondong/MSRA_proposal_wifi_log
	* 언어: Python + Jupyter notebook
	* 데이터 정보
		* 크롤링 데이터 기간
			* 크롤링 시작 ts: 1471870576 (08/22/2016 @12:56pm)
			* 크롤링 완료 ts: 1440568069 (08/26/2015 @ 5:47am)
		* Statistics
			* 로그 수: 3,962,136개 (DF를 Pickle로 저장: 430.2MB)
			* Unique device IDs: 1,526,073개
* 수: 데이터 프레임 분석, revisit count에 대한 sup. learning 모델 만들기
	* xgboost(end-to-end gradient boosting lib), keras(dnn) 이용
	* 참고: https://www.kaggle.com/c/talkingdata-mobile-user-demographics
		* 최근 Kaggle에 Kernel/Notebook 공유가 활발해서, 다른 데이터 분석가들의 코드를 참고하기 좋음(주로 R, Python).
	* Sup. learn. model
		* 일단 raw data를 feature로 이용
	* 평가 방법 결정
		* Log-loss (sci-kit learn)
* 목: 추가 데이터 크롤링, stay-point을 define하고 그에 맞는 feature 생성
* 금: 미팅 준비

* 8/29 - 9/2:
	* Supervised_model(basic_features).ipynb, revisit_predict.py: 이 코드는 device_id를 90:10로 나눈 후 train(features+revisit_count)를 이용하여 xgboost로(train-valid) 트레이닝한 후, test의 features(현재까지의 모든 로그)를 받아서 revisit_count를 예측하는 regression 모델이었다.  사실 이미 몇번 방문했는지까지 알고 있는 데이터의 feature를 다시 잡은 후(revisit_count를 feature 만드는 데 쓰진 않았지만), 예측하는 모델이기 때문에 실전에서 쓰이기가 어렵다.  
	* 내가 생각하는 실전에서의 예측: 어떤 사람이 지금까지 이런 식의 방문 패턴이 있는데, 앞으로 우리 마켓에 더 방문할 것인가?  
		* 두 번째 모델은 이렇게 만들어야 할 듯
		* 시간을 쪼개기 
			* 예를 들면: 2015년 8-12월 데이터로 2016 1-4월에 재 방문한지/안 한지를 트레이닝함, 그 후 2016년 1-4월 데이터를 이용해 5-8월의 재방문을 예측하기: 얼마나 예측이 잘 맞았는지 테스트
	*  Feature 몇 개 더 만들기 (우선 진행)
		* 교수님이 슬라이드에 언급한 stay time features
			* # of stay points (완료)
			* variance of the stay times at each stay points  (완료)
			* transit patterns(temporarily annotated sequence)  -- 어려움  
			* group movement patterns  -- 어려움 
	* 추가 데이터 크롤링 (8월 31일까지 - 완료)
	* 데이터 분석 결과물 result/result.json에 저장하도록 함(코드 실행 시 결과 데이터 누락되거나 삭제되는 것 방지).

* 9/5 - 9/9:

* 9/20: proposal 듀 



### Repository 내용 서머리
* 데이터
	* 781(코오롱 문정직영점), 786(커스텀멜로우 홍대점) 크롤링 후 pickle화 완료 - DataFrame으로 이용.
	* 일단 내 맥북에만 저장, 깃헙엔 공유 안함.
* 코드 예제(Jupyter Notebook)
	* crawler.ipynb: 데이터 크롤러(json 크롤링 및 pickle 생성)
	* basic_analysis.ipynb: 기본 분석 (데이터 프레임 갖고 놀기)
	* Supervised_model(basic_features).ipynb: 스크립트 짜기 전 xgboost 모델 구현 + toy example 분석
	* group_pattern.ipynb: 로그를 바탕으로 유저의 group movement pattern을 구현하고자 함 (20%, 완전 초기 단계)
	* final_experiment_result_analysis.ipynb: 실험 run 결과를 가지고 plot 등을 할 예정.
* 코드 스크립트(code)
	* revisit_predict.py: 7개의 feature를 바탕으로 xgboost - regression 모델을 돌리고, 실험 파라미터 및 결과를 저장 
	* 디테일 다큐먼트 : [링크](/code/explanation.md)
* 실험 결과물(result)
	* result.json: revisit_predict.py를 돌릴 때마다 해당 run의 파라미터 및 score 등을 업데이트.
* Documents
	* [documents.md 참고](/documents/documents.md)




