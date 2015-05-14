#!/bin/bash
host="$1"
scp -i ../../RubymineProjects/actionRocket/taxi2.pem ./test/pizza/background/* ec2-user@$host:/home/ec2-user/gemtest/ruby-giflib/test/pizza/background/
scp -i ../../RubymineProjects/actionRocket/taxi2.pem ./test/pizza/days/* ec2-user@$host:/home/ec2-user/gemtest/ruby-giflib/test/pizza/days/
scp -i ../../RubymineProjects/actionRocket/taxi2.pem ./test/pizza/hours/* ec2-user@$host:/home/ec2-user/gemtest/ruby-giflib/test/pizza/hours/
scp -i ../../RubymineProjects/actionRocket/taxi2.pem ./test/pizza/minutes/* ec2-user@$host:/home/ec2-user/gemtest/ruby-giflib/test/pizza/minutes/
scp -i ../../RubymineProjects/actionRocket/taxi2.pem ./test/pizza/seconds/* ec2-user@$host:/home/ec2-user/gemtest/ruby-giflib/test/pizza/seconds/
