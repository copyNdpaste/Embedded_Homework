Youtube(link)https://www.youtube.com/watch?v=iSTFlWjIbVc

생산자와 소비자 쓰레드를 생성
생산자는 락을 걸고 값을 생성한다. count가 MAX라면 소비자가 empty 신호를 보낼 때까지 대기한다. empty 신호가 오면 mutex를 푼다.
put으로 버퍼에 생성한 값을 집어 넣는다.

소비자는 락을 걸고 count가 0이면 생산자가 fill 신호를 보낼 때까지 대기한다. fill을 받으면 해야할 동작을 하거나 get으로 값을 얻어온다.

첫번째 과제에선 DB연동이 잘 되었으나 두번째 과제에서는 쓰레드가 꼬인건지 DB server에 문제가 있는 건지 DB 연동이 되지 않았다.
50-server.cnf 파일에 들어가서 패킷 최대 허용량과 응답 대기 시간(?)을 늘렸으나 실패했다.
이미 잘 사용하고 있는 클라우드 서버의 IP를 입력해도 소용이 없었다.
