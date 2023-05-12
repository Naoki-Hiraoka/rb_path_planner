#ifndef MULTICONTACT_LOCOMOTION_PLANNER_H
#define MULTICONTACT_LOCOMOTION_PLANNER_H

#include <ik_constraint2/ik_constraint2.h>
#include <choreonoid_viewer/choreonoid_viewer.h>
#include <multicontact_locomotion_planner/RobotState.h>
#include <multicontact_locomotion_planner/CnoidStateSpace.h>
#include <multicontact_locomotion_planner/RBRRTStateValidityChecker.h>
#include <ik_constraint2_distance_field/ik_constraint2_distance_field.h>
#include <prioritized_inverse_kinematics_solver2/prioritized_inverse_kinematics_solver2.h>

namespace multicontact_locomotion_planner{

  class RobotIKInfo {
  public:
    std::vector<std::shared_ptr<ik_constraint2::IKConstraint> > selfConstraints; // 自己干渉, 関節上下限
    std::vector<std::shared_ptr<ik_constraint2_distance_field::DistanceFieldCollisionConstraint> > envConstraints; // 環境干渉
    std::vector<std::shared_ptr<ik_constraint2::IKConstraint> > nominalConstraints; // reset-manip-pose. 常にisSatisfied = trueであること
    prioritized_inverse_kinematics_solver2::IKParam pikParam; // 接触遷移を伴わないcontact breakability check, 及び, subGoal到達に用いる
    std::vector<std::shared_ptr<prioritized_qp_base::Task> > tasks;
    RobotIKInfo() {
      pikParam.we = 1e2; // 逆運動学が振動しないこと優先. 1e0だと不安定. 1e3だと大きすぎる
      pikParam.maxIteration = 100; // max iterationに達するか、convergeしたら終了する. isSatisfiedでは終了しない. ゼロ空間でreference angleに可能な限り近づけるタスクがあるので. 1 iterationで0.5msくらいかかるので、stateを1つ作るための時間の上限が見積もれる. 一見、この値を小さくすると早くなりそうだが、goalSampling時に本当はgoalに到達できるのにその前に返ってしまうことで遅くなることがあるため、少ないiterationでも収束するように他のパラメータを調整したほうがいい
      pikParam.minIteration = 100;
      pikParam.checkFinalState = true; // ゼロ空間でreference angleに可能な限り近づけるタスクのprecitionは大きくして、常にsatisfiedになることに注意
      pikParam.calcVelocity = false; // 疎な軌道生成なので、velocityはチェックしない
      pikParam.convergeThre = 5e-2; // 要パラチューン. IKConsraintのmaxErrorより小さくないと、収束誤判定する. maxErrorが5e-2の場合、5e-2だと大きすぎる. 5e-3だと小さすぎて時間がかかる. ikのwe, wn, wmax, maxErrorといったパラメータと連動してパラチューンせよ.
      pikParam.pathOutputLoop = 5;
    }
  public:
    bool solveFullbodyIK(const std::vector<cnoid::LinkPtr>& variables, // 0: variables
                         const std::unordered_map<std::string, std::shared_ptr<Contact> >& currentContacts,
                         const std::vector<std::shared_ptr<ik_constraint2::IKConstraint> >& targetConstraints,
                         std::shared_ptr<std::vector<std::vector<double> > >& path
                         );
  };

  class MLPParam {
  public:
    int debugLevel = 0; // 0: no debug message. 1: time measure. 2: internal state
    std::shared_ptr<choreonoid_viewer::Viewer> viewer = nullptr;
    unsigned int drawLoop = 100; // スレッド0が、drawLoopに一回描画する

    unsigned int threads = 1;

    cnoid::BodyPtr robot;
    cnoid::BodyPtr abstractRobot;
    cnoid::BodyPtr horizontalRobot;
    std::vector<std::pair<cnoid::LinkPtr, cnoid::LinkPtr> > horizontals;
    std::unordered_map<std::string, std::shared_ptr<EndEffector> > endEffectors;
    std::unordered_map<std::string, std::shared_ptr<Mode> > modes;
    std::shared_ptr<RobotIKInfo> robotIKInfo = std::make_shared<RobotIKInfo>();



    double subGoalDistance = 0.5;

  };

  bool solveMLP(const cnoid::BodyPtr currentRobot,
                const std::shared_ptr<Environment>& environment,
                const std::unordered_map<std::string, std::shared_ptr<Contact> >& currentContacts,
                const std::string& prevSwingEEF,
                const std::vector<std::pair<std::vector<double>, std::string> >& targetRootPath, // angle, mode
                const std::vector<cnoid::LinkPtr>& variables, // 0: variables
                std::vector<RobotState>& outputPath, // variablesの次元に対応

                const MLPParam& param
                );

  bool solveContactBreakabilityIK(const std::vector<cnoid::LinkPtr>& variables, // 0: variables
                                  const std::unordered_map<std::string, std::shared_ptr<Contact> >& currentContacts,
                                  const std::unordered_map<std::string, std::shared_ptr<Contact> >& swingContacts,
                                  const std::shared_ptr<Contact>& breakContact,
                                  std::shared_ptr<std::vector<std::vector<double> > >& path
                                  ){
    return true;
  }

  bool isInsideReachiability(const std::vector<double>& subGoal,
                             const std::shared_ptr<EndEffector>& endEffector,
                             const std::shared_ptr<Contact>& contact){
    return true;
  }

  bool solveSwingTrajectory(const std::vector<cnoid::LinkPtr>& variables, // 0: variables
                            const std::unordered_map<std::string, std::shared_ptr<Contact> >& swingContacts,
                            const std::vector<double>& subGoal,
                            const std::shared_ptr<EndEffector>& targetEEF,
                            std::shared_ptr<std::vector<std::vector<double> > >& path
                            ){
    return true;
  }


};

#endif